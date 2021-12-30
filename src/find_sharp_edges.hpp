#pragma once

#include <Eigen/Dense>
#include <cassert>
#include <vector>

#include "Mesh.hpp"
#include "parallel_task.hpp"

template <typename Bool>
void find_sharp_edges(const Mesh& m, double min, double max,
                      std::vector<Bool>& eflags) {
  assert(eflags.size() == m.num_edges());

  const auto task = [&](auto, auto e, auto end) {
    for (; e < end; ++e) {
      auto f0 = m.e2f[e * 2];
      auto f1 = m.e2f[e * 2 + 1];

      if (f1 == -1) {
        eflags[e] = 2;
        continue;
      }

      Eigen::Vector3d n0{&m.fn[f0 * 3]};
      Eigen::Vector3d n1{&m.fn[f1 * 3]};

      // Assume normals are normalized.
      auto cos = n0.dot(n1);
      if (cos > min && cos < max) eflags[e] = true;
    }
  };

  utl::parallel_task(utl::NUM_HARDWARE_THREADS, 0, m.num_edges(), task);
}