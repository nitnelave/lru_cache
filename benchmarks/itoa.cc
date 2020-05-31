#include <random>
#include <sstream>
#include <string>
#include <vector>

#include "adapters.h"
#include "catch_util.h"
#include "lru_cache/lru_cache.h"

namespace lru_cache {

std::string ToString(int v) {
  std::stringstream out;
  out << v;
  return std::move(out).str();
}

template <size_t Value>
struct IntWrapper {};

template <size_t Value>
using Iterations = IntWrapper<Value>;

template <size_t Value>
using CacheSize = IntWrapper<Value>;

template <typename MissRate /* in percentage */, typename BenchSize,
          typename CacheSize>
struct BenchOptions;

template <size_t MissRate /* in percentage */, size_t BenchSize,
          size_t CacheSize>
struct BenchOptions<IntWrapper<MissRate>, IntWrapper<BenchSize>,
                    IntWrapper<CacheSize>> {
  static constexpr size_t MISS_RATE = MissRate;
  static_assert(MISS_RATE >= 0 && MISS_RATE < 100);
  static constexpr size_t BENCH_SIZE = BenchSize;
  static constexpr size_t CACHE_SIZE = CacheSize;
  static constexpr double MISS_RATIO = MISS_RATE / 100.0;
  static constexpr size_t MAX_VALUE =
      CACHE_SIZE + (CACHE_SIZE * MISS_RATIO / (1 - MISS_RATIO));
};

template <typename BenchSize, typename CacheSize>
using LowMissRateBenchOptions =
    BenchOptions<IntWrapper<5>, BenchSize, CacheSize>;

template <typename BenchSize, typename CacheSize>
using HighMissRateBenchOptions =
    BenchOptions<IntWrapper<50>, BenchSize, CacheSize>;

TEMPLATE_PRODUCT_TEST_CASE("Benchmark itoa", "[benchmarks]",
                           (LowMissRateBenchOptions, HighMissRateBenchOptions),
                           ((Iterations<1'000>, CacheSize<100>),
                            (Iterations<100'000>, CacheSize<10'000>))) {
  static constexpr size_t CACHE_SIZE = TestType::CACHE_SIZE;
  std::vector<int> keys(TestType::BENCH_SIZE);
  std::mt19937 gen(0);  // Using constant seed.
  std::uniform_int_distribution<> dist(0, TestType::MAX_VALUE);
  std::generate(keys.begin(), keys.end(), [&]() { return dist(gen); });
  BENCHMARK("baseline") {
    size_t total_size = 0;
    for (int key : keys) {
      total_size += ToString(key).size();
    }
    return total_size;
  };
  BENCHMARK("nitnelave/lru_cache/dynamic") {
    auto cache = make_dynamic_lru_cache<int, std::string>(CACHE_SIZE, ToString);
    size_t total_size = 0;
    for (int key : keys) {
      total_size += cache[key].size();
    }
    return total_size;
  };
  BENCHMARK("nitnelave/lru_cache/dynamic_reserve") {
    auto cache = make_dynamic_lru_cache<int, std::string>(CACHE_SIZE, ToString);
    cache.reserve(CACHE_SIZE);
    size_t total_size = 0;
    for (int key : keys) {
      total_size += cache[key].size();
    }
    return total_size;
  };
  BENCHMARK("nitnelave/lru_cache/static") {
    auto cache = make_static_lru_cache<int, std::string, uint32_t, CACHE_SIZE>(ToString);
    size_t total_size = 0;
    for (int key : keys) {
      total_size += cache[key].size();
    }
    return total_size;
  };
  BENCHMARK("nitnelave/lru_cache/node") {
    auto cache = make_node_lru_cache<int, std::string>(CACHE_SIZE, ToString);
    size_t total_size = 0;
    for (int key : keys) {
      total_size += cache[key].size();
    }
    return total_size;
  };
  BENCHMARK("lamerman/lrucache") {
    auto cache =
        make_lamerman_lru_cache<int, std::string, CACHE_SIZE>(ToString);
    size_t total_size = 0;
    for (int key : keys) {
      total_size += cache[key].size();
    }
    return total_size;
  };
  BENCHMARK("mohaps/LRUCache11") {
    auto cache = make_mohaps_lru_cache<int, std::string, CACHE_SIZE>(ToString);
    size_t total_size = 0;
    for (int key : keys) {
      total_size += cache[key].size();
    }
    return total_size;
  };
  BENCHMARK("vpetrigo/caches") {
    auto cache =
        make_vpetrigo_lru_cache<int, std::string, CACHE_SIZE>(ToString);
    size_t total_size = 0;
    for (int key : keys) {
      total_size += cache[key].size();
    }
    return total_size;
  };
  BENCHMARK("goldsborough/caches") {
    auto cache =
        make_goldsborough_lru_cache<int, std::string, CACHE_SIZE>(ToString);
    size_t total_size = 0;
    for (int key : keys) {
      total_size += cache[key].size();
    }
    return total_size;
  };
}
}  // namespace lru_cache
