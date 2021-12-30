#pragma once

#include <string_view>

#include "Mesh.hpp"

void readOBJ(std::string_view filepath, Mesh& m);
