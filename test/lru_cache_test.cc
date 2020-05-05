#include <iostream>

#include "catch_util.h"
#include "lru_cache.h"
#include "range_matcher_util.h"

using ::lru_cache::make_dynamic_lru_cache;

using ::Catch::Equals;

TEST_CASE("LRU cache drops LRU entries", "[LRU][dynamic]") {
  std::vector<int> dropped;
  std::vector<int> fetched;
  auto my_lru_cache = make_dynamic_lru_cache<int, int>(
      uint16_t{3},
      [&](int i) {
        fetched.push_back(i);
        return i;
      },
      [&](int key, int val) { dropped.push_back(key); });
  auto LruRangeEquals = &RangeEquals<decltype(my_lru_cache)>;
  REQUIRE(my_lru_cache.size() == 0);
  REQUIRE(my_lru_cache.empty());
  SECTION("Fetching fewer elements than the cache doesn't drop anything") {
    REQUIRE(my_lru_cache[1] == 1);
    REQUIRE(my_lru_cache[2] == 2);
    REQUIRE(my_lru_cache[3] == 3);
    REQUIRE(dropped.empty());
    REQUIRE_THAT(fetched, Equals(std::vector<int>{1, 2, 3}));
    SECTION("Fetching one more element drops the first") {
      REQUIRE(my_lru_cache[4] == 4);
      REQUIRE_THAT(fetched, Equals(std::vector<int>{1, 2, 3, 4}));
      REQUIRE_THAT(dropped, Equals(std::vector<int>{1}));
    }
    SECTION("Fetching an element already there doesn't fetch anything") {
      REQUIRE(my_lru_cache[1] == 1);
      REQUIRE(dropped.empty());
      REQUIRE_THAT(fetched, Equals(std::vector<int>{1, 2, 3}));
      SECTION("Fetching one more element drops the second") {
        REQUIRE(my_lru_cache[4] == 4);
        REQUIRE_THAT(fetched, Equals(std::vector<int>{1, 2, 3, 4}));
        REQUIRE_THAT(dropped, Equals(std::vector<int>{2}));
        REQUIRE_THAT(my_lru_cache, LruRangeEquals({{4, 4}, {1, 1}, {3, 4}}));
      }
    }
  }
  /*
  my_lru_cache[1];
  my_lru_cache[4];
  my_lru_cache[5];
  const auto& cache = my_lru_cache;
  for (const auto& kv : cache) {
    std::cout << "Key: " << kv.first << ", Value: " << kv.second << '\n';
  }
  */
}
