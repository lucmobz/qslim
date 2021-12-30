#include "readOBJ.hpp"

#include <array>
#include <cstdio>
#include <iostream>  // debug
#include <string_view>
#include <vector>

#include "Mesh.hpp"

static void parse_vertex(const char* line, Mesh& m) {
  std::array<double, 3> x;

  if (std::sscanf(line, " v %lf %lf %lf", &x[0], &x[1], &x[2]) != 3) {
    std::fprintf(stderr, "ERROR: Bad vertex format.\n");
    std::exit(EXIT_FAILURE);
  }

  m.v.insert(m.v.end(), x.begin(), x.end());
}

static void parse_texture(const char* line, Mesh& m) {
  std::array<double, 2> tx;

  if (std::sscanf(line, " vt %lf %lf", &tx[0], &tx[1]) != 2) {
    std::fprintf(stderr, "ERROR: Bad texture format.\n");
    std::exit(EXIT_FAILURE);
  }

  m.t.insert(m.t.end(), tx.begin(), tx.end());
}

static void parse_normal(const char* line, Mesh& m) {
  std::array<double, 3> nx;

  if (std::sscanf(line, " vn %lf %lf %lf", &nx[0], &nx[1], &nx[2]) != 3) {
    std::fprintf(stderr, "ERROR: Bad normal format.\n");
    std::exit(EXIT_FAILURE);
  }

  m.n.insert(m.n.end(), nx.begin(), nx.end());
}

static auto parse_indices(const char* p, int& offset) {
  // Order is vertex, texture, normal.
  std::array<int, 3> i{0, 0, 0};

  // Match in sscanf the trailing whitespaces to move the offset to the null
  // character if the line has ended (skipping \r\n). Also the order of pattern
  // matching matters, first more specific v/t/n than less v/t to avoid
  // premature matching. TODO: This may not be the best way to this honestly
  // have to measure perf.
  if (std::sscanf(p, " %d/%d/%d %n", &i[0], &i[1], &i[2], &offset) == 3) {
  } else if (std::sscanf(p, " %d/%d %n", &i[0], &i[1], &offset) == 2) {
  } else if (std::sscanf(p, " %d//%d %n", &i[0], &i[2], &offset) == 2) {
  } else if (std::sscanf(p, " %d %n", &i[0], &offset) == 1) {
  } else {
    // WARNING: This does not catch all errors.
    std::fprintf(stderr, "ERROR: Bad index format.\n");
    std::exit(EXIT_FAILURE);
  }
  return i;
}

static void parse_face(const char* line, Mesh& m) {
  ++line;
  int offset = 0;

  auto i0 = parse_indices(line, offset);
  line += offset;

  // Face must at least be a triangle.
  if (*line == '\0') {
    std::fprintf(stderr, "ERROR: Face has too few indices.\n");
    std::exit(EXIT_FAILURE);
  }

  auto i1 = parse_indices(line, offset);
  line += offset;

  // Face must at least be a triangle.
  if (*line == '\0') {
    std::fprintf(stderr, "ERROR: Face has too few indices.\n");
    std::exit(EXIT_FAILURE);
  }

  // Here all trailing whitespaces have to be consumed to check for line ending.
  while (*line != '\0') {
    auto i2 = parse_indices(line, offset);
    line += offset;

    // Perform simple fan triangulation and zero-based reindexing.
    if (i0[0] > 0)
      m.f2v.insert(m.f2v.end(), {i0[0] - 1, i1[0] - 1, i2[0] - 1});
    else if (i0[0] < 0) {
      int nv = m.v.size() / 3;
      m.f2v.insert(m.f2v.end(), {i0[0] + nv, i1[0] + nv, i2[0] + nv});
    } else {
      std::fprintf(stderr, "ERROR: Bad face format.\n");
      std::exit(EXIT_FAILURE);
    }

    if (i0[1] > 0)
      m.f2t.insert(m.f2t.end(), {i0[1] - 1, i1[1] - 1, i2[1] - 1});
    else if (i0[1] < 0) {
      int nt = m.t.size() / 3;
      m.f2t.insert(m.f2t.end(), {i0[1] + nt, i1[1] + nt, i2[1] + nt});
    }

    if (i0[2] > 0)
      m.f2n.insert(m.f2n.end(), {i0[2] - 1, i1[2] - 1, i2[2] - 1});
    else if (i0[2] < 0) {
      int nn = m.n.size() / 3;
      m.f2n.insert(m.f2n.end(), {i0[2] + nn, i1[2] + nn, i2[2] + nn});
    }

    // Rotate.
    i1 = i2;
  }
}

void readOBJ(std::string_view filepath, Mesh& m) {
  std::FILE* file = std::fopen(filepath.data(), "rb");

  char line[512];

  if (file) {
    while (std::fgets(line, 512, file)) {
      switch (line[0]) {
        case 'v':
          switch (line[1]) {
            case ' ':
            case '\t':
              parse_vertex(line, m);
              break;
            case 't':
              parse_texture(line, m);
              break;
            case 'n':
              parse_normal(line, m);
              break;
          }
          break;
        case 'f':
          parse_face(line, m);
          break;
      }
    }
  } else
    std::fprintf(stderr, "WARNING: Could not open \"%s\".\n", filepath.data());

  std::fclose(file);
}