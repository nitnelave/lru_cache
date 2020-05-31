#ifndef LRU_CACHE_BENCHMARKS_GOLDSBOROUGH_ADAPTER_
#define LRU_CACHE_BENCHMARKS_GOLDSBOROUGH_ADAPTER_

#include "lru/lru.hpp"

namespace lru_cache
{

template<typename Key, typename Value, size_t Size, typename ValueProvider>
class GoldsboroughLruCache {
 public:
  GoldsboroughLruCache(ValueProvider callback) : callback_(std::move(callback)) {}

  const Value& operator[](const Key& key) {
    if (cache_.contains(key)) {
      return cache_.lookup(key);
    }
    return cache_.emplace(key, callback_(key)).second->value();
  }

 private:
  ::LRU::Cache<Key, Value> cache_{Size};
  ValueProvider callback_;
};

template <typename Key, typename Value, size_t Size, typename ValueProvider>
GoldsboroughLruCache<Key, Value, Size, ValueProvider>
make_goldsborough_lru_cache(ValueProvider provider) {
  return {std::move(provider)};
}

}  // namespace lru_cache

#endif  // LRU_CACHE_BENCHMARKS_GOLDSBOROUGH_ADAPTER_



