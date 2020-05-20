// LRU cache based on a vector.
//
// Pros:
//  - only uses as much memory as needed.
// Cons:
//  - reallocation of the vector contents.
#ifndef LRU_CACHE_DYNAMIC_LRU_CACHE_H_
#define LRU_CACHE_DYNAMIC_LRU_CACHE_H_

#include <unordered_map>

#include "lru_cache_impl.h"
#include "vector_node_container.h"

namespace lru_cache {

// Options for a dynamic LRU cache with a maximum size.
// The index_type should be an unsigned integer.
template <typename Key, typename Value, typename index_type = uint16_t,
          bool by_access_order = true>
struct DynamicLruCacheOptions {
  using IndexType = index_type;

  static_assert(std::numeric_limits<IndexType>::is_integer,
                "IndexType should be an integer.");
  static_assert(!std::numeric_limits<IndexType>::is_signed,
                "IndexType should be unsigned.");

  using Map = std::unordered_map<Key, IndexType>;

  using NodeContainer =
      VectorNodeContainer<internal::Node<Key, Value, IndexType>>;

  static constexpr bool ByAccessOrder = by_access_order;
};

// An LRU cache based on a vector that will grow until the max_size.
template <typename Key, typename Value, typename ValueProvider,
          typename DroppedEntryCallback = void (*)(Key, Value)>
class DynamicLruCache
    : public internal::LruCacheImpl<
          DynamicLruCache<Key, Value, ValueProvider, DroppedEntryCallback>, Key,
          Value, DynamicLruCacheOptions<Key, Value>, ValueProvider,
          DroppedEntryCallback> {
  using Base = typename DynamicLruCache::Impl;
  friend Base;
  using options_type = DynamicLruCacheOptions<Key, Value>;
  using IndexType = typename options_type::IndexType;
  using NodeContainer = typename options_type::NodeContainer;
  using Map = typename options_type::Map;

 public:
  static constexpr IndexType MAX_REPRESENTABLE_SIZE =
      std::numeric_limits<IndexType>::max() - 1;
  // The maximum size should be at most one less than the maximum representable
  // integer.
  DynamicLruCache(IndexType max_size, ValueProvider value_provider,
                  DroppedEntryCallback dropped_entry_callback =
                      internal::no_op_dropped_entry_callback<Key, Value>)
      : Base(std::move(value_provider), std::move(dropped_entry_callback)),
        max_size_(max_size) {
    assert(max_size <= MAX_REPRESENTABLE_SIZE);
  }

  IndexType max_size() const { return max_size_; }

  void reserve(IndexType size) {
    assert(size <= max_size());
    nodes_.reserve(size);
  }

 protected:
  NodeContainer& node_container() { return nodes_; }
  Map& map() { return map_; }
  const Map& map() const { return map_; }

  IndexType index_of(const Key& key) const {
    auto it = map_.find(key);
    if (it != map_.end()) {
      return it->second;
    }
    return NodeContainer::INVALID_INDEX;
  }

 private:
  const IndexType max_size_;
  NodeContainer nodes_;
  Map map_;
};

// Factory function for a dynamic LRU cache.
// The maximum size should be at most one less than the maximum representable
// integer.
template <typename Key, typename Value, typename ValueProvider,
          typename DroppedEntryCallback = void (*)(Key, Value)>
DynamicLruCache<Key, Value, ValueProvider, DroppedEntryCallback>
make_dynamic_lru_cache(
    typename DynamicLruCacheOptions<Key, Value>::IndexType max_size,
    ValueProvider v,
    DroppedEntryCallback c =
        internal::no_op_dropped_entry_callback<Key, Value>) {
  return {max_size, v, c};
}

}  // namespace lru_cache

#endif  // LRU_CACHE_DYNAMIC_LRU_CACHE_H_
