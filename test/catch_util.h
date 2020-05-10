#ifndef LRU_CACHE_TEST_CATCH_UTIL_
#define LRU_CACHE_TEST_CATCH_UTIL_

template <typename T1, typename T2>
std::ostream &operator<<(std::ostream &os, const std::pair<T1, T2> &value) {
  os << '{' << value.first << ", " << value.second << '}';
  return os;
}

#include <catch2/catch.hpp>

#endif // LRU_CACHE_TEST_CATCH_UTIL_
