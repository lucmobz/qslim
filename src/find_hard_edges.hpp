#pragma once

#include <cassert>
#include <vector>

#include "Mesh.hpp"

// A hard edge has discontinuous normal indices/coordinates (first remove
// redundant normals from the buffer).
// clang-format off
//    f0
//   /  \ 
// n00__n01
// n10  n11
//   \  /
//    f1
// clang-format on
template <bool BY_COORDINATES = false, typename Bool>
void find_hard_edges(const Mesh& m, std::vector<Bool>& eflags) {
  assert(eflags.size() == m.num_edges());

  for (auto e = 0; e < m.num_edges(); ++e) {
    auto v0 = m.e2v[e * 2];
    auto v1 = m.e2v[e * 2 + 1];
    auto f0 = m.e2f[e * 2];
    auto f1 = m.e2f[e * 2 + 1];

    // In case of boundary edge set as hard edge.
    if (f1 == -1) {
      eflags[e] = 2;
      continue;
    }

    // Find the correct indices in the face.
    const auto* v00 = &m.f2v[f0 * 3];
    const auto* v01 = &m.f2v[f0 * 3];
    const auto* n00 = &m.f2n[f0 * 3];
    const auto* n01 = &m.f2n[f0 * 3];

    while (*v00 != v0) ++v00, ++n00;
    while (*v01 != v1) ++v01, ++n01;

    assert(v00 < &m.f2v[f0 * 3] + 3);
    assert(v01 < &m.f2v[f0 * 3] + 3);

    const auto* v10 = &m.f2v[f1 * 3];
    const auto* v11 = &m.f2v[f1 * 3];
    const auto* n10 = &m.f2n[f1 * 3];
    const auto* n11 = &m.f2n[f1 * 3];

    while (*v10 != v0) ++v10, ++n10;
    while (*v11 != v1) ++v11, ++n11;

    assert(v10 < &m.f2v[f1 * 3] + 3);
    assert(v11 < &m.f2v[f1 * 3] + 3);

    // This piece is only included if BY_COORDINATES=true.
    if constexpr (BY_COORDINATES) {
      // WARNING: This is not very efficient to have functor here everytime,
      // hope the compiler inlines this (if c++20 use constexpr version).
      const auto equal = [](const auto* n0, const auto* n1) {
        return std::equal(n0, n0 + 3, n1);
      };
      // In case of discontinuous normal coordinates set as hard edge.
      if (!equal(&m.n[*n00 * 3], &m.n[*n10 * 3]) ||
          !equal(&m.n[*n01 * 3], &m.n[*n11 * 3])) {
        eflags[e] = true;
      }
    } else if (*n00 != *n10 || *n01 != *n11) {
      // In case of discontinuous normal indices set as hard edge.
      eflags[e] = true;
    }
  }
}
