#pragma once

#include <robin_hood.h>  // Better unordered_map than STL.

#include <cmath>
#include <cstdint>
#include <vector>

// Test if a mesh is edge manifold by only looking at the face to vertex
// connectivity.
template <typename Index>
bool is_edge_manifold(const std::vector<Index>& f2v,
                      const std::vector<char>& fdel) {
  // Set up hash table.
  static_assert(sizeof(Index) <= 4);

  constexpr auto hash = [](Index v0, Index v1) {
    return static_cast<uint64_t>(v0) << 32 | static_cast<uint64_t>(v1);
  };

  robin_hood::unordered_map<uint64_t, int> map;

  // Loop over faces and for each unordered edge count how many times it
  // appears.
  auto num_faces = f2v.size() / 3;
  for (auto f = 0; f < num_faces; ++f) {
    if (fdel[f]) continue;

    auto v0 = f2v[f * 3];
    auto v1 = f2v[f * 3 + 1];
    auto v2 = f2v[f * 3 + 2];

    ++map[hash(std::min(v0, v1), std::max(v0, v1))];
    ++map[hash(std::min(v1, v2), std::max(v1, v2))];
    ++map[hash(std::min(v2, v0), std::max(v2, v0))];
  }

  // If an edge appears in more than 2 faces the mesh is non-manifold.
  for (auto [id, count] : map)
    if (count > 2) return false;

  return true;
}
