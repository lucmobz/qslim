#pragma once

#include <algorithm>
#include <cassert>
#include <vector>

#include "Mesh.hpp"

// WARNING: Highly inefficent.
static bool test_collapse_shared_neighbors(
    const Mesh& m, int e, const std::vector<char>& is_boundary_edge) {
  assert(!m.edel[e]);

  auto v0 = m.e2v[e * 2];
  auto v1 = m.e2v[e * 2 + 1];

  assert(v0 != v1);
  assert(!m.vdel[v0]);
  assert(!m.vdel[v1]);

  auto num_shared = 0;
  for (auto v : m.v2v[v0]) {
    if (m.vdel[v] || v == v1) continue;
    for (auto vv : m.v2v[v1]) {
      if (m.vdel[vv] || vv == v0) continue;
      if (v == vv) ++num_shared;
    }
  }

  assert(num_shared >= 1);

  return (!is_boundary_edge[e] && num_shared < 3) ||
         (is_boundary_edge[e] && num_shared < 2);
}