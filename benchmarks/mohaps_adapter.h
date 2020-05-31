#ifndef LRU_CACHE_BENCHMARKS_MOHAPS_ADAPTER_
#define LRU_CACHE_BENCHMARKS_MOHAPS_ADAPTER_

#include "LRUCache11.hpp"

namespace lru_cache {

template <typename Key, typename Value, size_t Size, typename ValueProvider>
class MohapsLruCache {
 public:
  MohapsLruCache(ValueProvider callback) : callback_(std::move(callback)) {}

  const Value& operator[](const Key& key) {
    if (!cache_.contains(key)) {
      Value v = callback_(key);
      cache_.insert(key, std::move(v));
    }
    return cache_.get(key);
  }

  const Value* get_or_null(const Key& key) {
    if (!cache_.contains(key)) return nullptr;
    return &cache_.get(key);
  }

  void insert(const Key& key, Value v) { cache_.insert(key, std::move(v)); }

 private:
  ::lru11::Cache<Key, Value> cache_{Size};
  ValueProvider callback_;
};

template <typename Key, typename Value, size_t Size,
          typename ValueProvider =
              decltype(&internal::throwing_value_producer<Key, Value>)>
MohapsLruCache<Key, Value, Size, ValueProvider> make_mohaps_lru_cache(
    ValueProvider provider = internal::throwing_value_producer<Key, Value>) {
  return {std::move(provider)};
}

}  // namespace lru_cache

#endif  // LRU_CACHE_BENCHMARKS_MOHAPS_ADAPTER_
