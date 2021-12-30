#include "writeOBJ.hpp"

#include <cassert>
#include <cstdio>
#include <iostream>  // debug
#include <string_view>

void writeOBJ(std::string_view filepath, Mesh& m) {
  assert(m.num_vertices() > 0);
  assert(m.num_faces() > 0);

  std::FILE* file = std::fopen(filepath.data(), "wb");

  if (file) {
    for (auto v = 0; v < m.num_vertices(); ++v) {
      std::fprintf(file, "v %lf %lf %lf\n", m.v[v * 3], m.v[v * 3 + 1],
                   m.v[v * 3 + 2]);
    }

    // for (auto t = 0; t < m.num_texture(); ++t) {
    //   std::fprintf(file, "vt %lf %lf %lf\n", m.t[t * 3], m.t[t * 3 + 1],
    //                m.t[t * 3 + 2]);
    // }

    // for (auto n = 0; n < m.num_normals(); ++n) {
    //   std::fprintf(file, "vn %lf %lf %lf\n", m.n[n * 3], m.n[n * 3 + 1],
    //                m.n[n * 3 + 2]);
    // }

    if (m.num_texture() == 0 && m.num_normals() == 0) {
      for (auto f = 0; f < m.num_faces(); ++f) {
        std::fprintf(file, "f %d %d %d\n", m.f2v[f * 3] + 1,
                     m.f2v[f * 3 + 1] + 1, m.f2v[f * 3 + 2] + 1);
      }
    } else if (m.num_texture() != 0 && m.num_normals() == 0) {
      for (auto f = 0; f < m.num_faces(); ++f) {
        std::fprintf(file, "f %d/%d %d/%d %d/%d\n", m.f2v[f * 3] + 1,
                     m.f2t[f * 3] + 1, m.f2v[f * 3 + 1] + 1,
                     m.f2t[f * 3 + 1] + 1, m.f2v[f * 3 + 2] + 1,
                     m.f2t[f * 3 + 2] + 1);
      }
    } else if (m.num_texture() == 0 && m.num_normals() != 0) {
      for (auto f = 0; f < m.num_faces(); ++f) {
        std::fprintf(file, "f %d//%d %d//%d %d//%d\n", m.f2v[f * 3] + 1,
                     m.f2n[f * 3] + 1, m.f2v[f * 3 + 1] + 1,
                     m.f2n[f * 3 + 1] + 1, m.f2v[f * 3 + 2] + 1,
                     m.f2n[f * 3 + 2] + 1);
      }
    } else {
      for (auto f = 0; f < m.num_faces(); ++f) {
        std::fprintf(file, "f %d/%d/%d %d/%d/%d %d/%d/%d\n", m.f2v[f * 3] + 1,
                     m.f2t[f * 3] + 1, m.f2n[f * 3] + 1, m.f2v[f * 3 + 1] + 1,
                     m.f2t[f * 3 + 1] + 1, m.f2n[f * 3 + 1] + 1,
                     m.f2v[f * 3 + 2] + 1, m.f2t[f * 3 + 2] + 1,
                     m.f2n[f * 3 + 2] + 1);
      }
    }
  } else {
    std::fprintf(stderr, "WARNING: Could not open \"%s\".\n", filepath.data());
  }
  std::fclose(file);
}