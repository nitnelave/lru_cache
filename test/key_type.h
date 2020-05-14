#ifndef LRU_CACHE_TEST_KEY_TYPE_H_
#define LRU_CACHE_TEST_KEY_TYPE_H_

#include <functional>

struct KeyType {
  KeyType() = default;
  KeyType(int i) : value(i) {}
  int value;

  template <typename H>
  friend H AbslHashValue(H h, const KeyType& n) {
    return H::combine(std::move(h), n.value);
  }

  bool operator==(const KeyType& other) const { return value == other.value; }
  bool operator!=(const KeyType& other) const { return !(*this == other); }
};

namespace std {
template <>
struct hash<KeyType> {
  std::size_t operator()(const KeyType& k) const {
    return hash<int>()(k.value);
  }
};
}  // namespace std

std::ostream& operator<<(std::ostream& os, const KeyType& key) {
  return os << key.value;
}

#endif  // LRU_CACHE_TEST_KEY_TYPE_H_
