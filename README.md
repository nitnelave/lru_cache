# LRU cache

A fast, header-only, generic C++ 17 [LRU cache][1] library, with customizable backend.

## Example usage

### Cache calls to an expensive function

```c++
#include "lru_cache/lru_cache.h"

int very_expensive_function(int key) {
  // Do a network call to get the result.
  return 42;
}

int main() {
  // Keep up to 100 calls. The type of the cache is deduced from the function.
  auto memoized = lru_cache::memoize_function(100, very_expensive_function);
  for (int i = 0; i < 1000; ++i) {
    // Automatically calls very_expensive_function if the key is not in the
    // cache.
    int result = memoized(i);
    do_something_with(result);
  }
}
```

### Memoize a recursive function

```c++
#include "lru_cache/lru_cache.h"

template <typename Cache>
size_t cached_fibo(int n, Cache& cache) {
  if (n < 2) return 1;
  // Get the element, or null. This updates the most recent access list.
  const size_t* maybe = cache.get_or_null(n);
  if (maybe) return *maybe;
  size_t result = cached_fibo(n - 1, cache) + cached_fibo(n - 2, cache);
  // Insert into the cache.
  cache.insert(n, result);
  return result;
}

int main() {
  // Cache 10 elements.
  auto cache = lru_cache::make_cache<int, size_t>(10);
  // Quick computation.
  size_t result = cached_fibo(60, cache);
}
```

## Configurable backends

There are 3 configurations available out of the box:

  - `DynamicLruCache`, consisting of an `std::unordered_map` of key to index,
    and a `std::vector` to hold the values and the linked list of last accesses.
  - `StaticLruCache`, similar except that the vector is replaced by a
    `std::array`-like structure, allowing you to prevent reallocation, and hold
    the memory for the values on the stack if you want.
  - `NodeLruCache`, the default, consisting of an
    [`absl::node_hash_set`](https://abseil.io/docs/cpp/guides/container#abslnode_hash_map-and-abslnode_hash_set)
    containing the key, the value and the linked list in the nodes.

The last one is the main innovation of this repository, and the fastest
implementation.

However, the backends are quite flexible, and it's relatively easy to switch a
map for another, or implement an alternative. You just need to provide a
container with the expected interface (see comments at the top of
[lru_cache_impl.h](lru_cache/lru_cache_impl.h)). Some cache implementations
actually let you change the type of the containers in an "Options" type that
you can pass in.

For instance, if you had a fixed-size map that doesn't allocate, it would be
easy to plug it in the `StaticLruCache` and get a no-allocation cache.

## Value provider and dropped entry callback

When using the memoizing cache, any missing key is resolved by calling the
function-like object provided at construction time.

Moreover, if needed, you can pass a function that will be called with any
deleted key. This allows you to have custom cleanup, keep some statistics and
so on.

## Benchmarks

The provided implementation were compared with several LRU cache
implementations gathered from GitHub.

  - [lamerman/cpp-lru-cache](https://github.com/lamerman/cpp-lru-cache)
  - [mohaps/lrucache11](https://github.com/mohaps/lrucache11)
  - [vpetrigo/caches](https://github.com/vpetrigo/caches) (Note that this one
    includes a mutex, so the comparison is unfair).
  - [goldsborough/lru-cache](https://github.com/goldsborough/lru-cache)

For the benchmarks, the mean is taken, with 100 samples, a minimum confidence
interval of 0.95, and 100ms of warmup.

The benchmarks are run on a i7-3770 (3.4 GHz) with 16G RAM.

### Random access

This benchmark initializes an array with random values chosen from a bigger
range than the max cache size. The range of the random values can be modified
to test different miss rates. Here we are experimenting with a 5% miss rate and
a 50% miss rate.

The task is to convert the given integer to a string. The baseline doesn't use
any caching.

The numbers are `miss rate`/`cache size`/`queries`.

| Benchmark                   | 5%/100/1k (μs) | 5%/10k/100k (ms) | 50%/100/1k (μs) | 50%/10k/100k (ms) |
|-----------------------------|----------------|------------------|-----------------|-------------------|
| baseline                    | 352            | 36.3             | 355             | 36.1              |
| nitnelave/lru_cache/dynamic | 78.5           | 9.04             | 272             | 29.7              |
| nitnelave/lru_cache/static  | 71.8           | 8.78             | 256             | 28.9              |
| nitnelave/lru_cache/node    | 66.9           | **8.20**         | **241**         | **26.0**          |
| lamerman/lrucache           | 85.2           | 9.26             | 271             | 31.4              |
| mohaps/LRUCache11           | **61.7**       | 9.40             | 260             | 32.0              |
| vpetrigo/caches             | 128.5          | 15.6             | 365             | 42.3              |
| goldsborough/caches         | 77.2           | 9.64             | 262             | 31.4              |

### Fibonacci

The task is to compute the _n_th Fibonacci number using the naive recursive
formula:

```
fibo(n) = fibo(n - 1) + fibo(n - 2)
```

This usually has a `O(2^n)` complexity, but we can drastically reduce the
runtime with caching.

The cache size was set to 10 elements.

The baseline was not run because it would take too long. We stopped at 92
because it's the biggest element that fits in 64 bits.

| Benchmark                   | fibo(60) (μs) | fibo(80) (μs) | fibo(92) (μs) |
|-----------------------------|---------------|---------------|---------------|
| nitnelave/lru_cache/dynamic | 4.56          | 6.09          | 7.01          |
| nitnelave/lru_cache/static  | 4.27          | 5.96          | 6.60          |
| nitnelave/lru_cache/node    | **3.15**      | **4.30**      | **4.91**      |
| lamerman/lrucache           | 6.11          | 8.29          | 9.49          |
| mohaps/LRUCache11           | 6.04          | 7.90          | 9.38          |
| vpetrigo/caches             | 12.2          | 16.4          | 18.7          |
| goldsborough/caches         | 5.45          | 7.52          | 8.40          |

[1]: http://go/link/wiki/Cache_replacement_policies#Least_recently_used_(LRU)
