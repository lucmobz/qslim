#pragma once

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <iterator>
#include <string_view>
#include <vector>

#include "Mesh.hpp"

static void writeVTK_edge_patch(std::string_view filepath, const Mesh& m,
                                int e) {
  assert(!m.edel[e]);

  auto v0 = m.e2v[e * 2];
  auto v1 = m.e2v[e * 2 + 1];
  assert(!m.vdel[v0]);
  assert(!m.vdel[v1]);

  auto f0 = m.e2f[e * 2];
  assert(f0 != -1);
  auto f1 = m.e2f[e * 2 + 1];

  std::vector<int> vs;

  for (auto vv : m.v2v[v0]) {
    if (m.vdel[vv]) continue;
    vs.push_back(vv);
  }

  for (auto vv : m.v2v[v1]) {
    if (m.vdel[vv]) continue;
    vs.push_back(vv);
  }

  std::sort(vs.begin(), vs.end());
  vs.erase(std::unique(vs.begin(), vs.end()), vs.end());

  std::vector<int> fs;

  for (auto ff : m.v2f[v0]) {
    if (m.vdel[ff]) continue;
    fs.push_back(ff);
  }

  for (auto ff : m.v2f[v1]) {
    if (m.vdel[ff]) continue;
    fs.push_back(ff);
  }

  std::sort(fs.begin(), fs.end());
  fs.erase(std::unique(fs.begin(), fs.end()), fs.end());

  std::FILE* file = std::fopen(filepath.data(), "wb");
  if (file) {
    std::fprintf(file,
                 "# vtk DataFile Version 3.0\nData\nASCII\nDATASET POLYDATA\n");
    std::fprintf(file, "POINTS %d double\n", vs.size());
    for (auto vv : vs)
      std::fprintf(file, "%lf %lf %lf\n", m.v[vv * 3], m.v[vv * 3 + 1],
                   m.v[vv * 3 + 2]);

    std::fprintf(file, "POLYGONS %d %d\n", fs.size(), 4 * fs.size());
    for (auto ff : fs) {
      // Get the local indices.
      std::fprintf(file, "3 ");
      for (auto i = 0; i < 3; ++i) {
        auto v = std::distance(
            vs.begin(), std::find(vs.begin(), vs.end(), m.f2v[ff * 3 + i]));
        std::fprintf(file, "%d ", v);
      }
      std::fprintf(file, "\n");
    }
  }
  std::fclose(file);
}