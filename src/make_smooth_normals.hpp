#pragma once

#include <Eigen/Dense>
#include <cassert>
#include <cmath>
#include <iostream>

#include "Mesh.hpp"
#include "parallel_task.hpp"

void make_smooth_normals(Mesh& m) {
  m.n.resize(m.num_vertices() * 3, 0.0);
  m.f2n = m.f2v;

  // WARNING: Assume face normals are normalized and non-zero. We are assuming
  // the surface is smooth everywhere.
  for (auto f = 0; f < m.num_faces(); ++f) {
    auto v0 = m.f2v[f * 3];
    auto v1 = m.f2v[f * 3 + 1];
    auto v2 = m.f2v[f * 3 + 2];

    Eigen::Vector3d x0{&m.v[v0 * 3]};
    Eigen::Vector3d x1{&m.v[v1 * 3]};
    Eigen::Vector3d x2{&m.v[v2 * 3]};

    auto s0 = (x2 - x1).normalized();
    auto s1 = (x0 - x2).normalized();
    auto s2 = (x1 - x0).normalized();

    auto theta0 = std::acos(-s2.dot(s1));
    auto theta1 = std::acos(-s0.dot(s2));
    auto theta2 = std::acos(-s1.dot(s0));

    Eigen::Map<Eigen::Vector3d>{&m.n[v0 * 3]} +=
        m.fa[f] * m.fa[f] * Eigen::Map<Eigen::Vector3d>{&m.fn[f * 3]};
    Eigen::Map<Eigen::Vector3d>{&m.n[v1 * 3]} +=
        m.fa[f] * m.fa[f] * Eigen::Map<Eigen::Vector3d>{&m.fn[f * 3]};
    Eigen::Map<Eigen::Vector3d>{&m.n[v2 * 3]} +=
        m.fa[f] * m.fa[f] * Eigen::Map<Eigen::Vector3d>{&m.fn[f * 3]};
  }

  const auto normalize_normals = [&](auto, auto v, auto end) {
    for (; v < end; ++v) {
      Eigen::Map<Eigen::Vector3d> n{&m.n[v * 3]};
      auto norm = n.norm();
      assert(norm != 0.0);
      if (norm > 0.0) n /= norm;
    }
  };

  utl::parallel_task(utl::NUM_HARDWARE_THREADS, 0, m.num_vertices(),
                     normalize_normals);
}
