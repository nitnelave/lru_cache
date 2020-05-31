#ifndef LRU_CACHE_BENCHMARKS_LAMERMAN_ADAPTER_
#define LRU_CACHE_BENCHMARKS_LAMERMAN_ADAPTER_

#include <lrucache.hpp>

#include "lru_cache/default_providers.h"

namespace lru_cache {

template <typename Key, typename Value, size_t Size, typename ValueProvider>
class LamermanLruCache {
 public:
  LamermanLruCache(ValueProvider callback) : callback_(std::move(callback)) {}

  const Value& operator[](const Key& key) {
    if (!cache_.exists(key)) {
      Value v = callback_(key);
      cache_.put(key, v);
    }
    return cache_.get(key);
  }

  const Value* get_or_null(const Key& key) {
    if (!cache_.exists(key)) return nullptr;
    return &cache_.get(key);
  }

  void insert(const Key& key, Value v) { cache_.put(key, std::move(v)); }

 private:
  ::cache::lru_cache<Key, Value> cache_{Size};
  ValueProvider callback_;
};

template <typename Key, typename Value, size_t Size,
          typename ValueProvider =
              decltype(&internal::throwing_value_producer<Key, Value>)>
LamermanLruCache<Key, Value, Size, ValueProvider> make_lamerman_lru_cache(
    ValueProvider provider = internal::throwing_value_producer<Key, Value>) {
  return {std::move(provider)};
}

}  // namespace lru_cache

#endif  // LRU_CACHE_BENCHMARKS_LAMERMAN_ADAPTER_
