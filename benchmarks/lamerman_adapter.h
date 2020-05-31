#ifndef LRU_CACHE_BENCHMARKS_LAMERMAN_ADAPTER_
#define LRU_CACHE_BENCHMARKS_LAMERMAN_ADAPTER_

#include <lrucache.hpp>

namespace lru_cache
{

template<typename Key, typename Value, size_t Size, typename ValueProvider>
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

 private:
  ::cache::lru_cache<Key, Value> cache_{Size};
  ValueProvider callback_;
};

template <typename Key, typename Value, size_t Size, typename ValueProvider>
LamermanLruCache<Key, Value, Size, ValueProvider>
make_lamerman_lru_cache(ValueProvider provider) {
  return {std::move(provider)};
}

}  // namespace lru_cache

#endif  // LRU_CACHE_BENCHMARKS_LAMERMAN_ADAPTER_
