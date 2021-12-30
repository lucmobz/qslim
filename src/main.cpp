#include <iostream>
#include <string>
#include <tuple>
#include <vector>
#define _USE_MATH_DEFINES
#include <cmath>
// clang-format off
// This has to be before any boost/graph stuff.
#include <boost/graph/vector_as_graph.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/timer/timer.hpp>
// clang-format on

#include "Mesh.hpp"
#include "Quadric.hpp"
#include "WriterVTK.hpp"
#include "collapse_edge.hpp"
#include "compress_buffer.hpp"
#include "find_boundary_edges.hpp"
#include "find_buffer_duplicates.hpp"
#include "find_duplicate_faces.hpp"
#include "find_hard_edges.hpp"
#include "find_non_manifold_faces.hpp"
#include "find_sharp_edges.hpp"
#include "get_optimal_position.hpp"
#include "is_closed.hpp"
#include "is_edge_manifold.hpp"
#include "is_oriented.hpp"
#include "make_compressed.hpp"
#include "make_edge_heap.hpp"
#include "make_edges.hpp"
#include "make_face_normals_and_areas.hpp"
#include "make_smooth_normals.hpp"
#include "make_topology.hpp"
#include "make_vertex_quadric.hpp"
#include "make_vertex_quadrics.hpp"
#include "readOBJ.hpp"
#include "split_into_connected_components.hpp"
#include "test_collapse_boundaries.hpp"
#include "test_collapse_normal_flipping.hpp"
#include "test_collapse_shared_neighbors.hpp"
#include "writeOBJ.hpp"
#include "writeVTK_edge_patch.hpp"
#include "writeVTK_vertex_patch.hpp"

int main(int argc, char** argv) {
  boost::timer::auto_cpu_timer t;

  Mesh m;

  // Read original mesh.
  readOBJ(argv[1], m);

  // Pick a mesh.
  {
    // Big problems.
    {
      std::vector<char> duplicate_faces(m.num_faces(), false);
      find_duplicate_faces(m, duplicate_faces);
      std::cout << "Duplicate faces: "
                << std::count(duplicate_faces.begin(), duplicate_faces.end(),
                              true)
                << '\n';

      // Brutally remove duplicate faces.
      m.f2v.resize(3 * compress_buffer<3>(m.f2v.data(), m.num_faces(),
                                          duplicate_faces.data()));
      if (!m.t.empty())
        m.f2t.resize(3 * compress_buffer<3>(m.f2t.data(), m.num_faces(),
                                            duplicate_faces.data()));
      if (!m.n.empty())
        m.f2n.resize(3 * compress_buffer<3>(m.f2n.data(), m.num_faces(),
                                            duplicate_faces.data()));
    }

    // Mesh regularity.
    {
      std::cout << "Oriented: " << is_oriented(m.f2v) << '\n';
      std::vector<double> nmf(m.num_faces(), false);
      find_non_manifold_faces(m.f2v, nmf);
      std::cout << "Edge-manifold: "
                << (std::count(nmf.begin(), nmf.end(), true) == 0) << '\n';
      std::cout << "Non-manifold faces: "
                << std::count(nmf.begin(), nmf.end(), true) << '\n';

      // debug
      // {
      //   WriterVTK w;
      //   w.add_point_buffer(m.v.data(), m.num_vertices(), 3);
      //   w.add_cell_buffer(m.f2v.data(), m.num_faces(), WriterVTK::TRIANGLE);
      //   w.add_scalar_buffer(nmf.data(), nmf.size(), WriterVTK::CELL);
      //   w.write("out.vtk");
      // }
      // exit(1);

      // Brutally remove non manifold faces.
      m.f2v.resize(3 *
                   compress_buffer<3>(m.f2v.data(), m.num_faces(), nmf.data()));
      if (!m.t.empty())
        m.f2t.resize(
            3 * compress_buffer<3>(m.f2t.data(), m.num_faces(), nmf.data()));
      if (!m.n.empty())
        m.f2n.resize(
            3 * compress_buffer<3>(m.f2n.data(), m.num_faces(), nmf.data()));

      // Check for consistency.
      std::cout << "Oriented: " << is_oriented(m.f2v) << '\n';
      nmf.assign(m.num_faces(), false);
      find_non_manifold_faces(m.f2v, nmf);
      std::cout << "Edge-manifold: "
                << (std::count(nmf.begin(), nmf.end(), true) == 0) << '\n';
    }

    // Preprocess vertices.
#if 1
    {
      constexpr auto hash = [](const double* x) {
        const auto h0 = std::hash<double>{}(x[0]);
        const auto h1 = std::hash<double>{}(x[1]);
        const auto h2 = std::hash<double>{}(x[2]);

        return (h0 ^ (h1 << 1)) ^ h2;
      };

      std::vector<char> vflags(m.num_vertices(), false);
      std::vector<int> ind0(m.num_vertices(), -1);
      std::vector<int> ind1(m.num_vertices(), -1);

      find_buffer_duplicates<3>(hash, m.v.data(), m.num_vertices(),
                                vflags.data(), ind0.data());

      std::cout << "Vertex duplicates: "
                << std::count(vflags.begin(), vflags.end(), true) << '\n';

      m.v.resize(compress_buffer<3>(m.v.data(), m.num_vertices(), vflags.data(),
                                    ind1.data()) *
                 3);
      for (auto& v : m.f2v) v = ind1[ind0[v]];
    }
#endif

    // Mesh data.
    make_face_normals_and_areas(m);
    std::cout << "Zero area faces: "
              << std::count(m.fa.begin(), m.fa.end(), 0.0) << '\n';

    // Compute connectivities.
    make_topology(m);
    make_edges(m);
    m.fdel.assign(m.num_faces(), false);
    m.vdel.assign(m.num_vertices(), false);
    m.edel.assign(m.num_edges(), false);

    // Pick a connected component.
    std::vector<int> f2cc(m.num_faces());
    auto num_components = boost::connected_components(m.f2f, f2cc.data());
    std::cout << "Connected components: " << num_components << '\n';

    std::vector<Mesh> ms;
    {
      std::vector<std::vector<int>> cc2f(num_components);
      for (auto f = 0; f < m.num_faces(); ++f) cc2f[f2cc[f]].push_back(f);

      split_into_connected_components(m, cc2f, ms);
    }

    {
      auto cn = std::stoi(argv[2]);
      std::cout << "Component number: " << cn << '\n';
      m = ms[cn];
      make_topology(m);
      make_edges(m);
      m.vdel.assign(m.num_vertices(), false);
      m.fdel.assign(m.num_faces(), false);
      m.edel.assign(m.num_edges(), false);
    }
  }

  // debug
  {
    for (auto e = 0; e < m.num_edges(); ++e)
      if (m.e2f[e * 2] == -1) std::cout << "ERROR\n";
  }

  // Print data.
  std::cout << "Faces: " << m.num_faces() << '\n';
  std::cout << "Vertices: " << m.num_vertices() << '\n';
  std::cout << "Normals: " << m.num_normals() << '\n';
  std::cout << "Texture: " << m.num_texture() << '\n';

  std::vector<char> is_boundary_edge(m.num_edges(), false);
  find_boundary_edges(m, is_boundary_edge);
  std::cout << "Boundary edges: "
            << std::count(is_boundary_edge.begin(), is_boundary_edge.end(),
                          true)
            << '\n';

  std::vector<char> is_boundary_vertex(m.num_vertices(), false);
  for (auto e = 0; e < m.num_edges(); ++e) {
    if (is_boundary_edge[e]) {
      is_boundary_vertex[m.e2v[e * 2]] = true;
      is_boundary_vertex[m.e2v[e * 2 + 1]] = true;
    }
  }

  std::cout << "Boundary vertices: "
            << std::count(is_boundary_vertex.begin(), is_boundary_vertex.end(),
                          true)
            << '\n';

  std::cout << "Closed: " << is_closed(m) << '\n';

// Preprocess normals.
#if 1
  if (!m.n.empty()) {
    constexpr auto hash = [](const double* x) {
      const auto h0 = std::hash<double>{}(x[0]);
      const auto h1 = std::hash<double>{}(x[1]);
      const auto h2 = std::hash<double>{}(x[2]);

      return (h0 ^ (h1 << 1)) ^ h2;
    };

    std::vector<char> nflags(m.n.size() / 3, false);
    std::vector<int> ind0(m.n.size() / 3, -1);
    std::vector<int> ind1(m.n.size() / 3, -1);

    find_buffer_duplicates<3>(hash, m.n.data(), m.n.size() / 3, nflags.data(),
                              ind0.data());

    std::cout << "Normal duplicates: "
              << std::count(nflags.begin(), nflags.end(), true) << '\n';

    m.n.resize(compress_buffer<3>(m.n.data(), m.n.size() / 3, nflags.data(),
                                  ind1.data()) *
               3);
    m.n.shrink_to_fit();
    for (auto& n : m.f2n) n = ind1[ind0[n]];
  }
#endif

  // Mesh data.
  make_face_normals_and_areas(m);
  std::cout << "Zero area faces: " << std::count(m.fa.begin(), m.fa.end(), 0.0)
            << '\n';

#if 1
  // std::vector<double> eflags(m.num_edges(), false);

  // find_hard_edges<true>(m, eflags);
  // find_sharp_edges(m, std::cos(M_PI / 2 + 0.2), std::cos(M_PI / 2 - 0.2),
  //                  eflags);
  // std::cout << "Hard edges: "
  //           << std::count_if(eflags.begin(), eflags.end(),
  //                            [](auto x) { return x > 0.0; })
  //           << '\n';
#endif

  // Collapse.
  auto vq = make_vertex_quadrics(m, is_boundary_edge, is_boundary_vertex);

  using edge_info_t = std::tuple<double, int, int>;
  std::vector<std::tuple<double, int, int>> eh;
  std::vector<Eigen::Vector3d> xs;

  make_edge_heap(m, vq, eh, xs);

  constexpr auto cmp = [](const auto& l, const auto& r) {
    return std::get<0>(l) > std::get<0>(r);
  };

  std::vector<int> times(m.num_edges(), 0);

  {
    std::cout << "Starting.\n";
    boost::timer::auto_cpu_timer t;

    auto num_faces = m.num_faces();
    auto target_num_faces = std::max(4, std::stoi(argv[3]));

    for (auto i = 0; num_faces > target_num_faces && !eh.empty(); ++i) {
      const auto& [c, e, t] = eh.front();
      const auto* x = &(xs[e][0]);
      auto v0 = m.e2v[e * 2];
      auto v1 = m.e2v[e * 2 + 1];

      std::vector<std::tuple<int, Eigen::Vector3d, double>> ns;

      // Put less expensive test first.
      if (m.edel[e] || t < times[e] ||
          !test_collapse_boundaries(m, e, is_boundary_edge,
                                    is_boundary_vertex) ||
          !test_collapse_shared_neighbors(m, e, is_boundary_edge) ||
          !test_collapse_normal_flipping(m, e, x, std::cos(M_PI / 3.0), ns)) {
        // std::cerr << "WARNING: Collapse of edge " << e
        //           << " rejected. Deleted: " << static_cast<bool>(m.edel[e])
        //           << ". Time " << t << " vs " << times[e] << ".\n";
        std::pop_heap(eh.begin(), eh.end(), cmp);
        eh.pop_back();
      } else {
        // std::cout << "Edge: " << e << '\n';
        // std::cout << "Cost: " << c << '\n';
        // std::cout << "Position: " << xs[e].transpose() << '\n';

        // debug
        // writeVTK_edge_patch("edge_patch.vtk", m, e);
        collapse_edge(m, e, x, is_boundary_edge, is_boundary_vertex);
        // writeVTK_vertex_patch("vertex_patch.vtk", m, v0);

        // Update number of faces.
        num_faces -= is_boundary_edge[e] ? 1 : 2;

        // Update precomputed normals and areas.
        for (const auto& [f, n, a] : ns) {
          std::copy(&n[0], &n[0] + 3, &m.fn[f * 3]);
          m.fa[f] = a;
        }

        // Update quadric of surviving vertex by accumulating error.
        vq[v0].A += vq[v1].A;
        vq[v0].b += vq[v1].b;
        vq[v0].c += vq[v1].c;

        // Update queue and quadrics.
        std::pop_heap(eh.begin(), eh.end(), cmp);
        eh.pop_back();

        for (auto e : m.v2e[v0]) {
          if (m.edel[e]) continue;

          auto v1 = m.e2v[e * 2] == v0 ? m.e2v[e * 2 + 1] : m.e2v[e * 2];
          assert(!m.vdel[v1]);

          // Sum quadrics.
          Quadric q;
          q.A = vq[v0].A + vq[v1].A;
          q.b = vq[v0].b + vq[v1].b;
          q.c = vq[v0].c + vq[v1].c;

          // Compute position.
          Eigen::Vector3d x;
          if (!get_optimal_position(q, x)) {
            std::cout << "WARNING: Optimal position failed.\n";
            Eigen::Vector3d x0{&m.v[v0 * 3]};
            Eigen::Vector3d x1{&m.v[v1 * 3]};
            x = (x0 + x1) * 0.5;
          }

          xs[e] = x;
          ++times[e];
          eh.emplace_back(vq[v0](x), e, times[e]);
          std::push_heap(eh.begin(), eh.end(), cmp);
        }
      }
    }
  }

  // Output.
  std::cout << "Finished.\n";
  make_compressed(m);
  // make_topology(m);
  // make_edges(m);
  // make_face_normals_and_areas(m);
  // make_smooth_normals(m);

  std::cout << "Writing OBJ.\n";
  writeOBJ("out.obj", m);

  {
    std::cout << "Writing VTK.\n";
    WriterVTK w;
    w.add_point_buffer(m.v.data(), m.num_vertices(), 3);
    w.add_cell_buffer(m.f2v.data(), m.num_faces(), WriterVTK::TRIANGLE);
    w.write("out.vtk");
  }
}