#include "lru_cache/lru_cache.h"

#include <iostream>

#include "catch_util.h"
#include "key_type.h"
#include "range_matcher_util.h"

using ::Catch::Equals;
namespace lru_cache {

template <typename Key, typename Value, size_t Size>
struct DynamicLruCacheTester {
  template <typename ValueProducer,
            typename DroppedCallback =
                decltype(internal::no_op_dropped_entry_callback<Key, Value>)>
  static decltype(auto) new_cache(
      ValueProducer p,
      DroppedCallback d = internal::no_op_dropped_entry_callback<Key, Value>) {
    return make_dynamic_lru_cache<Key, Value>(Size, p, d);
  }
};

template <typename Key, typename Value, size_t Size>
struct StaticLruCacheTester {
  template <typename ValueProducer,
            typename DroppedCallback =
                decltype(internal::no_op_dropped_entry_callback<Key, Value>)>
  static decltype(auto) new_cache(
      ValueProducer p,
      DroppedCallback d = internal::no_op_dropped_entry_callback<Key, Value>) {
    return make_static_lru_cache<Key, Value, uint16_t, Size>(p, d);
  }
};

template <typename Key, typename Value, size_t Size>
struct NodeLruCacheTester {
  template <typename ValueProducer,
            typename DroppedCallback =
                decltype(internal::no_op_dropped_entry_callback<Key, Value>)>
  static decltype(auto) new_cache(
      ValueProducer p,
      DroppedCallback d = internal::no_op_dropped_entry_callback<Key, Value>) {
    return make_node_lru_cache<Key, Value>(Size, p, d);
  }
};

TEMPLATE_PRODUCT_TEST_CASE_SIG("LRU cache drops LRU entries", "[LRU]",
                               ((typename Key, typename Value, size_t Size),
                                Key, Value, Size),
                               (DynamicLruCacheTester, StaticLruCacheTester,
                                NodeLruCacheTester),
                               ((KeyType, int, 3))) {
  std::vector<int> dropped;
  std::vector<int> fetched;
  auto my_lru_cache = TestType::new_cache(
      [&](const KeyType& i) {
        fetched.push_back(i.value);
        return i.value;
      },
      [&](KeyType key, int val) { dropped.push_back(key.value); });
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
  SECTION(
      "Fetching two elements then accessing the first one keeps the "
      "pointers okay") {
    REQUIRE(my_lru_cache[1] == 1);
    REQUIRE(my_lru_cache[2] == 2);
    REQUIRE(my_lru_cache[1] == 1);
    REQUIRE(dropped.empty());
    REQUIRE_THAT(fetched, Equals(std::vector<int>{1, 2}));
    REQUIRE_THAT(my_lru_cache, LruRangeEquals({{1, 1}, {2, 2}}));
  }
}

TEMPLATE_PRODUCT_TEST_CASE_SIG("LRU cache works with move-only types", "[LRU]",
                               ((typename Key, typename Value, size_t Size),
                                Key, Value, Size),
                               (DynamicLruCacheTester, StaticLruCacheTester,
                                NodeLruCacheTester),
                               ((KeyType, std::unique_ptr<int>, 3))) {
  std::vector<int> dropped;
  auto my_lru_cache = TestType::new_cache(
      [](KeyType i) { return std::make_unique<int>(i.value); },
      [&](KeyType key, std::unique_ptr<int> val) {
        dropped.push_back(key.value);
      });
  SECTION("Fetching fewer elements than the cache doesn't drop anything") {
    REQUIRE(*my_lru_cache[1] == 1);
    REQUIRE(*my_lru_cache[2] == 2);
    REQUIRE(*my_lru_cache[3] == 3);
    REQUIRE(dropped.empty());
    SECTION("Fetching one more element drops the first") {
      REQUIRE(*my_lru_cache[4] == 4);
      REQUIRE_THAT(dropped, Equals(std::vector<int>{1}));
    }
  }
}

struct DestructionStats {
  int constructed = 0;
  int destroyed = 0;
};

struct DestructorCounter {
  DestructorCounter(DestructionStats& stats) : stats_(&stats) {
    stats.constructed++;
  }
  DestructorCounter(const DestructorCounter&) = delete;
  DestructorCounter& operator=(const DestructorCounter&) = delete;
  DestructorCounter(DestructorCounter&& other) = default;
  DestructorCounter& operator=(DestructorCounter&& other) = default;
  ~DestructorCounter() {
    if (stats_) stats_->destroyed++;
  }

 private:
  DestructionStats* stats_ = nullptr;
};

TEMPLATE_PRODUCT_TEST_CASE_SIG(
    "LRU cache works with non-default-constructible types", "[LRU]",
    ((typename Key, typename Value, size_t Size), Key, Value, Size),
    (DynamicLruCacheTester, StaticLruCacheTester, NodeLruCacheTester),
    ((KeyType, DestructorCounter, 3))) {
  DestructionStats stats;
  SECTION("No element get destroyed if none are constructed") {
    {
      auto my_lru_cache = TestType::new_cache(
          [&](KeyType i) { return DestructorCounter(stats); });
      REQUIRE(stats.constructed == 0);
    }
    REQUIRE(stats.constructed == 0);
    REQUIRE(stats.destroyed == 0);
  }
  SECTION("Fetching a few elements makes only them destroyed.") {
    {
      auto my_lru_cache = TestType::new_cache(
          [&](KeyType i) { return DestructorCounter(stats); });
      my_lru_cache[1];
      my_lru_cache[2];
      REQUIRE(stats.constructed == 2);
    }
    // Ideally we would check the number of destroyed items as well, but with
    // move constructors/move elision, that number is >2 and not stable between
    // the containers and optimization levels.
    REQUIRE(stats.constructed == 2);
  }
}

TEMPLATE_PRODUCT_TEST_CASE_SIG("LRU cache can be moved", "[LRU]",
                               ((typename Key, typename Value, size_t Size),
                                Key, Value, Size),
                               (DynamicLruCacheTester,
                                // StaticLruCacheTester is not moveable.
                                NodeLruCacheTester),
                               ((KeyType, int, 3))) {
  SECTION("No element get destroyed if none are constructed") {
    auto my_lru_cache = TestType::new_cache([](KeyType i) { return i.value; });
    auto second_cache = std::move(my_lru_cache);
  }
}

void drop_callback(KeyType k, int v) {}
int produce_callback(const KeyType& k) { return 3; }

struct ValueProducer {
  ValueProducer(int i) : val_(i) {}

  int operator()(const KeyType& key) { return val_; }

  int val_;
};

TEST_CASE("Explicit cache type") {
  // Dynamic.
  {
    DynamicLruCache<KeyType, int, std::function<int(const KeyType&)>>
        cache_function(42, [](const KeyType&) { return 3; });
    DynamicLruCache<KeyType, int, int (*)(const KeyType&)> cache(
        42, produce_callback);
    auto made_cache =
        make_dynamic_lru_cache<KeyType, int>(42, produce_callback);
    static_assert(std::is_same_v<decltype(cache), decltype(made_cache)>);

    DynamicLruCache<KeyType, int, ValueProducer> cache_producer =
        make_dynamic_lru_cache<KeyType, int>(42, ValueProducer{3});
  }
  // Static.
  {
    StaticLruCache<KeyType, int, uint8_t, 42,
                   std::function<int(const KeyType&)>>
        cache_function([](const KeyType&) { return 3; });
    StaticLruCache<KeyType, int, uint8_t, 42, int (*)(const KeyType&)> cache(
        produce_callback);
    auto made_cache =
        make_static_lru_cache<KeyType, int, uint8_t, 42>(produce_callback);
    static_assert(std::is_same_v<decltype(cache), decltype(made_cache)>);
    StaticLruCache<KeyType, int, uint8_t, 42, ValueProducer> cache_producer =
        make_static_lru_cache<KeyType, int, uint8_t, 42>(ValueProducer{3});
  }
  // Node.
  {
    NodeLruCache<KeyType, int, std::function<int(const KeyType&)>>
        cache_function(42, [](const KeyType&) { return 3; });
    NodeLruCache<KeyType, int, int (*)(const KeyType&)> cache(42,
                                                              produce_callback);
    auto made_cache = make_node_lru_cache<KeyType, int>(42, produce_callback);
    static_assert(std::is_same_v<decltype(cache), decltype(made_cache)>);

    NodeLruCache<KeyType, int, ValueProducer> cache_producer =
        make_node_lru_cache<KeyType, int>(42, ValueProducer{3});
  }
}

}  // namespace lru_cache
