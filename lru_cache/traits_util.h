#ifndef LRU_CACHE_TRAITS_UTIL_H_
#define LRU_CACHE_TRAITS_UTIL_H_

#include <limits>
#include <tuple>
#include <type_traits>

namespace lru_cache::internal {

template <typename F>
struct function_info : public function_info<decltype(&F::operator())> {};

template <typename ReturnType, typename... Args>
struct function_info<ReturnType (*)(Args...)> {
  using return_type = ReturnType;
  using args_type = std::tuple<Args...>;
};

template <typename ClassType, typename ReturnType, typename... Args>
struct function_info<ReturnType (ClassType::*)(Args...) const> {
  using return_type = ReturnType;
  using args_type = std::tuple<Args...>;
};

template <typename ClassType, typename ReturnType, typename... Args>
struct function_info<ReturnType (ClassType::*)(Args...)> {
  using return_type = ReturnType;
  using args_type = std::tuple<Args...>;
};

template <typename F>
using return_t = typename function_info<F>::return_type;

template <typename F>
using args_t = typename function_info<F>::args_type;

template <size_t N>
using index_type_for = std::conditional_t<
    (N < std::numeric_limits<uint8_t>::max()), uint8_t,
    std::conditional_t<
        (N < std::numeric_limits<uint16_t>::max()), uint16_t,
        std::conditional_t<(N < std::numeric_limits<uint32_t>::max()), uint32_t,
                           uint64_t>>>;

}  // namespace lru_cache::internal
#endif  // LRU_CACHE_TRAITS_UTIL_H_
