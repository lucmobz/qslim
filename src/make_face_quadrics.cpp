#include "make_face_quadrics.hpp"

#include <Eigen/Dense>
#include <iostream>
#include <tuple>
#include <vector>

#include "Mesh.hpp"
#include "Quadric.hpp"
#include "parallel_task.hpp"

auto make_face_quadrics(const Mesh& m) -> std::vector<Quadric> {
  assert(!m.fn.empty());
  assert(m.fn.size() % 3 == 0);

  std::vector<Quadric> fq(m.num_faces());

  const auto task = [&](auto, auto f, auto end) {
    for (; f < end; ++f) {
      assert(m.fa[f] != 0.0);
      Eigen::Map<const Eigen::Vector3d> n{&m.fn[f * 3]};
      Eigen::Map<const Eigen::Vector3d> x0{&m.v[m.f2v[f * 3] * 3]};
      assert(n != Eigen::Vector3d::Zero());
      fq[f] = make_quadric(n, x0);
    }
  };
  utl::parallel_task(utl::NUM_HARDWARE_THREADS, 0, m.num_faces(), task);

  return fq;
}
