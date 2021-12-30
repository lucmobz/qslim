#pragma once

#include <string_view>

#include "Mesh.hpp"

void writeOBJ(std::string_view filepath, Mesh& m);