#include "adapters.h"
#include "catch_util.h"
#include "lru_cache/lru_cache.h"

namespace lru_cache {

size_t fibo(int n) {
  if (n < 2) return 1;
  return fibo(n - 1) + fibo(n - 2);
}

template <typename Cache>
size_t cached_fibo(int n, Cache& cache) {
  if (n < 2) return 1;
  const size_t* maybe = cache.get_or_null(n);
  if (maybe) return *maybe;
  size_t result = cached_fibo(n - 1, cache) + cached_fibo(n - 2, cache);
  cache.insert(n, result);
  return result;
}

template <size_t N>
struct FiboValue {
  static constexpr size_t value = N;
};

TEMPLATE_TEST_CASE("Benchmark fibo", "[benchmarks]", FiboValue<60>, FiboValue<80>, FiboValue<92>) {
  static constexpr size_t CACHE_SIZE = 10;
  static constexpr size_t VALUE = TestType::value;
  BENCHMARK("nitnelave/lru_cache/dynamic") {
    auto cache = make_dynamic_lru_cache<int, size_t>(CACHE_SIZE);
    return cached_fibo(VALUE, cache);
  };
  BENCHMARK("nitnelave/lru_cache/dynamic_reserve") {
    auto cache = make_dynamic_lru_cache<int, size_t>(CACHE_SIZE);
    cache.reserve(CACHE_SIZE);
    return cached_fibo(VALUE, cache);
  };
  BENCHMARK("nitnelave/lru_cache/static") {
    auto cache = make_static_lru_cache<int, size_t, CACHE_SIZE>();
    return cached_fibo(VALUE, cache);
  };
  BENCHMARK("nitnelave/lru_cache/node") {
    auto cache = make_node_lru_cache<int, size_t>(CACHE_SIZE);
    return cached_fibo(VALUE, cache);
  };
  BENCHMARK("lamerman/lrucache") {
    auto cache = make_lamerman_lru_cache<int, size_t, CACHE_SIZE>();
    return cached_fibo(VALUE, cache);
  };
  BENCHMARK("mohaps/LRUCache11") {
    auto cache = make_mohaps_lru_cache<int, size_t, CACHE_SIZE>();
    return cached_fibo(VALUE, cache);
  };
  BENCHMARK("vpetrigo/caches") {
    auto cache = make_vpetrigo_lru_cache<int, size_t, CACHE_SIZE>();
    return cached_fibo(VALUE, cache);
  };
  BENCHMARK("goldsborough/caches") {
    auto cache = make_goldsborough_lru_cache<int, size_t, CACHE_SIZE>();
    return cached_fibo(VALUE, cache);
  };
}

}  // namespace lru_cache
