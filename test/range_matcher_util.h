#ifndef LRU_CACHE_TEST_RANGE_MATCHER_UTIL_H_
#define LRU_CACHE_TEST_RANGE_MATCHER_UTIL_H_
#include <vector>

#include "catch_util.h"

template <typename T> class RangeMatcher : public Catch::MatcherBase<T> {
public:
  using value_type = typename T::value_type;
  RangeMatcher(const std::vector<value_type> &expected)
      : vector_matcher_(expected) {}

  bool match(const T &actual) const override {
    std::vector<value_type> actual_vector(actual.begin(), actual.end());
    return vector_matcher_.match(actual_vector);
  }

  std::string describe() const override { return vector_matcher_.describe(); }

private:
  ::Catch::Matchers::Vector::EqualsMatcher<
      value_type, std::allocator<value_type>, std::allocator<value_type>>
      vector_matcher_;
};

template <typename T>
RangeMatcher<T>
RangeEquals(const std::vector<typename T::value_type> &expected) {
  return RangeMatcher<T>(expected);
}

#endif // LRU_CACHE_TEST_RANGE_MATCHER_UTIL_H_
