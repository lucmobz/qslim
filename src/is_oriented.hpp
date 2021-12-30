#pragma once

#include <robin_hood.h>  // Better unordered_map than STL.

#include <cstdint>
#include <vector>

// Test if a mesh has all faces oriented accordingly by only looking at the face
// to vertex connectivity.
template <typename Index>
bool is_oriented(const std::vector<Index>& f2v) {
  // Set up hash table.
  static_assert(sizeof(Index) <= 4);

  robin_hood::unordered_set<uint64_t> set;

  constexpr auto hash = [](Index v0, Index v1) {
    return (uint64_t)v0 << 32 | (uint64_t)v1;
  };

  // Loop over faces and for each ordered edge count how many times it
  // appears.
  auto num_faces = f2v.size() / 3;
  for (auto f = 0; f < num_faces; ++f) {
    auto v0 = f2v[3 * f];
    auto v1 = f2v[3 * f + 1];
    auto v2 = f2v[3 * f + 2];

    // If an ordered edge has already been inserted than there is a flipped
    // face.
    if (!set.insert(hash(v0, v1)).second) return false;
    if (!set.insert(hash(v1, v2)).second) return false;
    if (!set.insert(hash(v2, v0)).second) return false;
  }
  return true;
}
