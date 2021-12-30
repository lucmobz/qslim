#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <limits>
#include <numeric>
#include <queue>
#include <ranges>
#include <set>
#include <unordered_set>
#include <vector>

int main() {
  std::vector<int> v{1, 2, 3, 4, 5, 5, 2, 34, 14, 1};
  std::vector<int> i(v.size());
  std::iota(i.begin(), i.end(), 0);

  auto* p = v.data();
  auto* q = i.data();

  while (*p != 34) ++p, ++q;

  std::cout << *p << ' ' << *q << '\n';
}