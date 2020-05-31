#ifndef LRU_CACHE_BENCHMARKS_GOLDSBOROUGH_ADAPTER_
#define LRU_CACHE_BENCHMARKS_GOLDSBOROUGH_ADAPTER_

#include "lru/lru.hpp"
#include "lru_cache/default_providers.h"

namespace lru_cache {

template <typename Key, typename Value, size_t Size, typename ValueProvider>
class GoldsboroughLruCache {
 public:
  GoldsboroughLruCache(ValueProvider callback)
      : callback_(std::move(callback)) {}

  const Value& operator[](const Key& key) {
    if (cache_.contains(key)) {
      return cache_.lookup(key);
    }
    return cache_.emplace(key, callback_(key)).second->value();
  }

  const Value* get_or_null(const Key& key) {
    auto it = cache_.find(key);
    if (cache_.is_valid(it)) return &it->value();
    return nullptr;
  }

  void insert(const Key& key, Value v) { cache_.insert(key, std::move(v)); }

 private:
  ::LRU::Cache<Key, Value> cache_{Size};
  ValueProvider callback_;
};

template <typename Key, typename Value, size_t Size,
          typename ValueProvider =
              decltype(&internal::throwing_value_producer<Key, Value>)>
GoldsboroughLruCache<Key, Value, Size, ValueProvider>
make_goldsborough_lru_cache(
    ValueProvider provider = internal::throwing_value_producer<Key, Value>) {
  return {std::move(provider)};
}

}  // namespace lru_cache

#endif  // LRU_CACHE_BENCHMARKS_GOLDSBOROUGH_ADAPTER_
