#ifndef LRU_CACHE_BENCHMARKS_VPETRIGO_ADAPTER_
#define LRU_CACHE_BENCHMARKS_VPETRIGO_ADAPTER_

#include "cache.hpp"
#include "lru_cache_policy.hpp"

namespace lru_cache {

template <typename Key, typename Value, size_t Size, typename ValueProvider>
class VpetrigoLruCache {
 public:
  VpetrigoLruCache(ValueProvider callback) : callback_(std::move(callback)) {}

  const Value& operator[](const Key& key) {
    if (!cache_.Cached(key)) {
      Value v = callback_(key);
      cache_.Put(key, std::move(v));
    }
    return cache_.Get(key);
  }

  const Value* get_or_null(const Key& key) {
    if (!cache_.Cached(key)) return nullptr;
    return &cache_.Get(key);
  }

  void insert(const Key& key, Value v) { cache_.Put(key, std::move(v)); }

 private:
  ::caches::fixed_sized_cache<Key, Value, ::caches::LRUCachePolicy<Key>> cache_{
      Size};
  ValueProvider callback_;
};

template <typename Key, typename Value, size_t Size,
          typename ValueProvider =
              decltype(&internal::throwing_value_producer<Key, Value>)>
VpetrigoLruCache<Key, Value, Size, ValueProvider> make_vpetrigo_lru_cache(
    ValueProvider provider = internal::throwing_value_producer<Key, Value>) {
  return {std::move(provider)};
}

}  // namespace lru_cache

#endif  // LRU_CACHE_BENCHMARKS_VPETRIGO_ADAPTER_
