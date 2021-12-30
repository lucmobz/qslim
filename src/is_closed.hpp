#pragma once

#include <cassert>
#include <vector>

#include "Mesh.hpp"

static bool is_closed(const Mesh& m) {
  assert(m.num_edges() > 0);
  assert(!m.e2f.empty());
  for (auto e = 0; e < m.num_edges(); ++e)
    if (m.e2f[2 * e + 1] == -1) return false;
  return true;
}