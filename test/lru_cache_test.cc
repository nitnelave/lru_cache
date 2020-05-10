#include <iostream>

#include "lru_cache/lru_cache.h"

#include "catch_util.h"
#include "key_type.h"
#include "range_matcher_util.h"

using ::Catch::Equals;

using ValueType = int;

struct DynamicLruCacheTester {
  decltype(auto) new_cache(std::vector<int> &dropped,
                           std::vector<int> &fetched) {
    return ::lru_cache::make_dynamic_lru_cache<KeyType, ValueType>(
        uint16_t{3},
        [&](KeyType i) {
          fetched.push_back(i.value);
          return i.value;
        },
        [&](KeyType key, ValueType val) { dropped.push_back(key.value); });
  }
};

struct StaticLruCacheTester {
  decltype(auto) new_cache(std::vector<int> &dropped,
                           std::vector<int> &fetched) {
    return ::lru_cache::make_static_lru_cache<KeyType, ValueType, uint16_t, 3>(
        [&](KeyType i) {
          fetched.push_back(i.value);
          return i.value;
        },
        [&](KeyType key, ValueType val) { dropped.push_back(key.value); });
  }
};

struct NodeLruCacheTester {
  decltype(auto) new_cache(std::vector<int> &dropped,
                           std::vector<int> &fetched) {
    return ::lru_cache::make_node_lru_cache<KeyType, ValueType>(
        3,
        [&](KeyType i) {
          fetched.push_back(i.value);
          return i.value;
        },
        [&](KeyType key, ValueType val) { dropped.push_back(key.value); });
  }
};

TEMPLATE_TEST_CASE("LRU cache drops LRU entries", "[LRU]",
                   DynamicLruCacheTester, StaticLruCacheTester,
                   NodeLruCacheTester) {
  TestType tester;
  std::vector<int> dropped;
  std::vector<int> fetched;
  auto my_lru_cache = tester.new_cache(dropped, fetched);
  auto LruRangeEquals = &RangeEquals<decltype(my_lru_cache)>;
  REQUIRE(my_lru_cache.size() == 0);
  REQUIRE(my_lru_cache.empty());
  SECTION("Fetching fewer elements than the cache doesn't drop anything") {
    REQUIRE(my_lru_cache[1] == 1);
    REQUIRE(my_lru_cache[2] == 2);
    REQUIRE(my_lru_cache[3] == 3);
    REQUIRE(dropped.empty());
    REQUIRE_THAT(fetched, Equals(std::vector<int>{1, 2, 3}));
    REQUIRE_THAT(my_lru_cache, LruRangeEquals({{3, 3}, {2, 2}, {1, 1}}));
    SECTION("Fetching one more element drops the first") {
      REQUIRE(my_lru_cache[4] == 4);
      REQUIRE_THAT(fetched, Equals(std::vector<int>{1, 2, 3, 4}));
      REQUIRE_THAT(dropped, Equals(std::vector<int>{1}));
      REQUIRE_THAT(my_lru_cache, LruRangeEquals({{4, 4}, {3, 3}, {2, 2}}));
    }
    SECTION("Fetching an element already there doesn't fetch anything") {
      REQUIRE(my_lru_cache[1] == 1);
      REQUIRE(dropped.empty());
      REQUIRE_THAT(fetched, Equals(std::vector<int>{1, 2, 3}));
      REQUIRE_THAT(my_lru_cache, LruRangeEquals({{1, 1}, {3, 3}, {2, 2}}));
      SECTION("Fetching one more element drops the second") {
        REQUIRE(my_lru_cache[4] == 4);
        REQUIRE_THAT(fetched, Equals(std::vector<int>{1, 2, 3, 4}));
        REQUIRE_THAT(dropped, Equals(std::vector<int>{2}));
        REQUIRE_THAT(my_lru_cache, LruRangeEquals({{4, 4}, {1, 1}, {3, 3}}));
      }
    }
  }
  SECTION("Fetching two elements then accessing the first one keeps the "
          "pointers okay") {
    REQUIRE(my_lru_cache[1] == 1);
    REQUIRE(my_lru_cache[2] == 2);
    REQUIRE(my_lru_cache[1] == 1);
    REQUIRE(dropped.empty());
    REQUIRE_THAT(fetched, Equals(std::vector<int>{1, 2}));
    REQUIRE_THAT(my_lru_cache, LruRangeEquals({{1, 1}, {2, 2}}));
  }
}
