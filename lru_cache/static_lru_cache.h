// LRU cache based on a static array.
//
// Pros:
//  - no reallocation.
// Cons:
//  - uses the maximum memory from the start.
#ifndef LRU_CACHE_STATIC_LRU_CACHE_H_
#define LRU_CACHE_STATIC_LRU_CACHE_H_

#include "array_node_container.h"
#include "lru_cache_impl.h"
#include <unordered_map>

namespace lru_cache {

// Options for a static (fixed-size) LRU cache, of size N.
// The index_type should be an unsigned integer.
template <typename Key, typename Value, typename index_type, index_type N,
          bool by_access_order = true>
struct StaticLruCacheOptions {
  using IndexType = index_type;

  static_assert(std::numeric_limits<IndexType>::is_integer,
                "IndexType should be an integer.");
  static_assert(!std::numeric_limits<IndexType>::is_signed,
                "IndexType should be unsigned.");

  static constexpr IndexType MAX_SIZE =
      std::numeric_limits<IndexType>::max() - 1;
  static_assert(N <= MAX_SIZE);

  using Map = std::unordered_map<Key, IndexType>;

  using NodeContainer =
      ArrayNodeContainer<internal::Node<Key, Value, IndexType>, N>;

  static constexpr bool ByAccessOrder = by_access_order;
};

// An LRU cache based on a static, fixed-size storage (no realloc).
template <typename Key, typename Value, typename IndexType, IndexType N,
          typename ValueProvider,
          typename DroppedEntryCallback =
              decltype(internal::no_op_dropped_entry_callback<Key, Value>)>
class StaticLruCache
    : public internal::LruCacheImpl<
          StaticLruCache<Key, Value, IndexType, N, ValueProvider,
                         DroppedEntryCallback>,
          Key, Value, StaticLruCacheOptions<Key, Value, IndexType, N>,
          ValueProvider, DroppedEntryCallback> {
  using Base =
      internal::LruCacheImpl<StaticLruCache, Key, Value,
                             StaticLruCacheOptions<Key, Value, IndexType, N>,
                             ValueProvider, DroppedEntryCallback>;
  friend Base;

  using options_type = StaticLruCacheOptions<Key, Value, IndexType, N>;
  using NodeContainer = typename options_type::NodeContainer;
  using Map = typename options_type::Map;

public:
  StaticLruCache(ValueProvider value_provider,
                 DroppedEntryCallback dropped_entry_callback = {})
      : Base(std::move(value_provider), std::move(dropped_entry_callback)) {}

  IndexType max_size() const { return N; }

protected:
  NodeContainer &node_container() { return nodes_; }
  Map &map() { return map_; }
  const Map &map() const { return map_; }

  IndexType index_of(const Key &key) const {
    auto it = map_.find(key);
    if (it != map_.end()) {
      return it->second;
    }
    return NodeContainer::INVALID_INDEX;
  }

private:
  NodeContainer nodes_;
  Map map_;
};

// Factory function for a static LRU cache.
template <typename Key, typename Value, typename IndexType, IndexType N,
          typename ValueProvider,
          typename DroppedEntryCallback =
              decltype(internal::no_op_dropped_entry_callback<Key, Value>)>
StaticLruCache<Key, Value, IndexType, N, ValueProvider, DroppedEntryCallback>
make_static_lru_cache(ValueProvider v,
                      DroppedEntryCallback c =
                          internal::no_op_dropped_entry_callback<Key, Value>) {
  return {v, c};
}

} // namespace lru_cache

#endif // LRU_CACHE_STATIC_LRU_CACHE_H_
