#include "catch_util.h"
#include "key_type.h"
#include "lru_cache/lru_cache.h"
#include "pointer_matcher_util.h"
#include "range_matcher_util.h"

namespace lru_cache {

TEST_CASE("LRU cache without provider throws", "[LRU]") {
  auto cache = make_node_lru_cache<int, int>(3);
  auto LruRangeEquals = &RangeEquals<decltype(cache)>;
  REQUIRE(cache.size() == 0);
  REQUIRE(cache.empty());
  SECTION("Fetching an element throws") {
    REQUIRE_THROWS_WITH(cache[1], "LRU cache: Key not found: 1");
    REQUIRE(cache.size() == 0);
    REQUIRE(cache.empty());
  }
  SECTION("Inserting an element works") {
    REQUIRE(cache.insert(1, 10) == 10);
    REQUIRE(cache.get(1) == 10);
    REQUIRE(cache[1] == 10);
    SECTION("get_or_null works correctly") {
      REQUIRE_THAT(cache.get_or_null(1), PointsTo(10));
      REQUIRE(cache.get_or_null(2) == nullptr);
    }
    SECTION("get_or_null updates access order") {
      REQUIRE(cache.insert(2, 20) == 20);
      REQUIRE_THAT(cache, LruRangeEquals({{2, 20}, {1, 10}}));
      REQUIRE_THAT(cache.get_or_null(1), PointsTo(10));
      REQUIRE_THAT(cache, LruRangeEquals({{1, 10}, {2, 20}}));
    }
  }
}

}  // namespace lru_cache
