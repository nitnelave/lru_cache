#include <pthread.h>

#include <iostream>

#include "catch_util.h"
#include "lru_cache/lru_cache.h"
#include "range_matcher_util.h"

using ::Catch::Equals;

struct KeyType {
  KeyType() = default;
  KeyType(int i) : value(i) {}
  int value;

  template <typename H>
  friend H AbslHashValue(H h, const KeyType& n) {
    return H::combine(std::move(h), n.value);
  }

  bool operator==(const KeyType& other) const { return value == other.value; }
  bool operator!=(const KeyType& other) const { return !(*this == other); }
};

std::ostream& operator<<(std::ostream& os, const KeyType& key) {
  return os << key.value;
}

struct NodeLruCacheTester {
  decltype(auto) new_cache(std::vector<KeyType>& dropped,
                           std::vector<KeyType>& fetched) {
    return ::lru_cache::make_node_lru_cache<KeyType, int>(
        3,
        [&](KeyType i) {
          fetched.push_back(i);
          return i.value;
        },
        [&](KeyType key, int val) { dropped.push_back(key); });
  }
};

TEST_CASE("LRU cache drops LRU entries", "[LRU][dynamic]") {
  NodeLruCacheTester tester;
  std::vector<KeyType> dropped;
  std::vector<KeyType> fetched;
  auto my_lru_cache = tester.new_cache(dropped, fetched);
  auto LruRangeEquals = &RangeEquals<decltype(my_lru_cache)>;
  REQUIRE(my_lru_cache.size() == 0);
  REQUIRE(my_lru_cache.empty());
  SECTION("Fetching fewer elements than the cache doesn't drop anything") {
    REQUIRE(my_lru_cache[1] == 1);
    REQUIRE(my_lru_cache[2] == 2);
    REQUIRE(my_lru_cache[3] == 3);
    REQUIRE(dropped.empty());
    REQUIRE_THAT(fetched, Equals(std::vector<KeyType>{1, 2, 3}));
    REQUIRE_THAT(my_lru_cache, LruRangeEquals({{3, 3}, {2, 2}, {1, 1}}));
    SECTION("Fetching one more element drops the first") {
      REQUIRE(my_lru_cache[4] == 4);
      REQUIRE_THAT(fetched, Equals(std::vector<KeyType>{1, 2, 3, 4}));
      REQUIRE_THAT(dropped, Equals(std::vector<KeyType>{1}));
      REQUIRE_THAT(my_lru_cache, LruRangeEquals({{4, 4}, {3, 3}, {2, 2}}));
    }
    SECTION("Fetching an element already there doesn't fetch anything") {
      REQUIRE(my_lru_cache[1] == 1);
      REQUIRE(dropped.empty());
      REQUIRE_THAT(fetched, Equals(std::vector<KeyType>{1, 2, 3}));
      REQUIRE_THAT(my_lru_cache, LruRangeEquals({{1, 1}, {3, 3}, {2, 2}}));
      SECTION("Fetching one more element drops the second") {
        REQUIRE(my_lru_cache[4] == 4);
        REQUIRE_THAT(fetched, Equals(std::vector<KeyType>{1, 2, 3, 4}));
        REQUIRE_THAT(dropped, Equals(std::vector<KeyType>{2}));
        REQUIRE_THAT(my_lru_cache, LruRangeEquals({{4, 4}, {1, 1}, {3, 3}}));
      }
    }
  }
}
