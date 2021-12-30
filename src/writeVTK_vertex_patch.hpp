#pragma once

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <iterator>
#include <string_view>
#include <vector>

#include "Mesh.hpp"

static void writeVTK_vertex_patch(std::string_view filepath, const Mesh& m,
                                  int v) {
  assert(!m.vdel[v]);

  std::vector<int> vs;

  vs.push_back(v);
  for (auto vv : m.v2v[v]) {
    if (m.vdel[vv]) continue;
    vs.push_back(vv);
  }

  std::sort(vs.begin(), vs.end());
  vs.erase(std::unique(vs.begin(), vs.end()), vs.end());

  std::vector<int> fs;

  for (auto ff : m.v2f[v]) {
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