#pragma once

#include <algorithm>
#include <iterator>
#include <vector>

#include "Mesh.hpp"

// TODO: Problems with make_edges that cannot compute e2f if mesh is non
// manifold and so also cannot compute e2v.
template <typename Bool>
void find_non_manifold_faces_and_edges(const Mesh& m, std::vector<Bool>& fflags,
                                       std::vector<Bool>& eflags) {
  for (auto e = 0; e < m.num_edges(); ++e) {
    auto v0 = m.e2v[e * 2];
    auto v1 = m.e2v[e * 2 + 1];

    std::vector<int> fs;
    std::set_intersection(m.v2f[v0].begin(), m.v2f[v0].end(), m.v2f[v1].begin(),
                          m.v2f[v1].end(), std::back_inserter(fs));

    if (fs.size() > 2) {
      for (auto f : fs) fflags[f] = true;
      eflags[e] = true;
    }
  }
}