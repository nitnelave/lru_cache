#ifndef LRU_CACHE_TEST_POINTER_MATCHER_UTIL_H_
#define LRU_CACHE_TEST_POINTER_MATCHER_UTIL_H_
#include <vector>

#include "catch_util.h"

template <typename T>
class PointerMatcher : public Catch::MatcherBase<const T*> {
 public:
  PointerMatcher(const T& expected) : expected_(expected) {}

  bool match(const T* const& actual) const override {
    return actual != nullptr && *actual == expected_;
  }

  std::string describe() const override {
    std::ostringstream ss;
    ss << "is not null and points to " << expected_;
    return ss.str();
  }

 private:
  T expected_;
};

template <typename T>
PointerMatcher<T> PointsTo(const T& expected) {
  return PointerMatcher<T>(expected);
}

#endif  // LRU_CACHE_TEST_POINTER_MATCHER_UTIL_H_
