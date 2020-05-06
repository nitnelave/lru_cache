#ifndef LRU_CACHE_DYNAMIC_LRU_CACHE_H_
#define LRU_CACHE_DYNAMIC_LRU_CACHE_H_

#include "lru_cache_impl.h"
#include <unordered_map>
#include <vector>

namespace lru_cache {

struct DynamicLruCacheOptions {
  template <typename K, typename V, typename... A>
  using Map = std::unordered_map<K, V, A...>;

  template <typename V, typename... A> using Array = std::vector<V, A...>;

  using IndexType = uint16_t;

  static constexpr bool ByAccessOrder = true;
  static constexpr bool LRU = true;
};

template <typename Key, typename Value, typename ValueProvider,
          typename DroppedEntryCallback =
              decltype(internal::no_op_dropped_entry_callback<Key, Value>)>
class DynamicLruCache
    : public internal::LruCacheImpl<
          DynamicLruCache<Key, Value, ValueProvider, DroppedEntryCallback>, Key,
          Value, ValueProvider, DynamicLruCacheOptions, DroppedEntryCallback> {
  using Base =
      internal::LruCacheImpl<DynamicLruCache, Key, Value, ValueProvider,
                             DynamicLruCacheOptions, DroppedEntryCallback>;
  using IndexType = DynamicLruCacheOptions::IndexType;

public:
  DynamicLruCache(IndexType max_size, ValueProvider value_provider,
                  DroppedEntryCallback dropped_entry_callback = {})
      : Base(std::move(value_provider), std::move(dropped_entry_callback)),
        max_size_(max_size) {
    assert(max_size > 2);
  }

  IndexType max_size() const { return max_size_; }

protected:
  const IndexType max_size_;
};

template <typename Key, typename Value, typename ValueProvider,
          typename DroppedEntryCallback =
              decltype(internal::no_op_dropped_entry_callback<Key, Value>)>
DynamicLruCache<Key, Value, ValueProvider, DroppedEntryCallback>
make_dynamic_lru_cache(DynamicLruCacheOptions::IndexType max_size,
                       ValueProvider v,
                       DroppedEntryCallback c =
                           internal::no_op_dropped_entry_callback<Key, Value>) {
  return {max_size, v, c};
}

} // namespace lru_cache

#endif // LRU_CACHE_DYNAMIC_LRU_CACHE_H_
