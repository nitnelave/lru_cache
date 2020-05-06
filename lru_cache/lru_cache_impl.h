#ifndef LRU_CACHE_LRU_CACHE_IMPL_H_
#define LRU_CACHE_LRU_CACHE_IMPL_H_

#include <cassert>
#include <experimental/type_traits>
#include <iterator>
#include <type_traits>

namespace lru_cache::internal {

template <typename Key, typename Value>
static constexpr auto no_op_dropped_entry_callback = [](Key, Value) {};

template <typename Container, typename Value>
using has_emplace_back_t = decltype(
    std::declval<Container &>().emplace_back(std::declval<Value &&>()));

template <typename Container, typename Value>
constexpr bool has_emplace_back =
    std::experimental::is_detected_v<has_emplace_back_t, Container, Value>;

template <typename CRTPBase, typename Key, typename Value,
          typename ValueProvider, typename CacheOptions,
          typename DroppedEntryCallback =
              decltype(no_op_dropped_entry_callback<Key, Value>)>
class LruCacheImpl {
public:
  using IndexType = typename CacheOptions::IndexType;

  template <typename K, typename V, typename... A>
  using Map = typename CacheOptions::template Map<K, V, A...>;

  template <typename V, typename... A>
  using Array = typename CacheOptions::template Array<V, A...>;

  static constexpr bool ByAccessOrder = CacheOptions::ByAccessOrder;

  static constexpr bool LRU = CacheOptions::LRU;

  using value_type = std::pair<Key, Value>;

  static_assert(std::numeric_limits<IndexType>::is_integer,
                "IndexType should be an integer.");
  static_assert(!std::numeric_limits<IndexType>::is_signed,
                "IndexType should be unsigned.");
  static constexpr IndexType INVALID_INDEX =
      std::numeric_limits<IndexType>::max();
  static constexpr IndexType MAX_SIZE =
      std::numeric_limits<IndexType>::max() - 1;

  LruCacheImpl(ValueProvider value_provider,
               DroppedEntryCallback dropped_entry_callback = {})
      : value_provider_(std::move(value_provider)),
        dropped_entry_callback_(std::move(dropped_entry_callback)) {}

  decltype(auto) size() const { return index_map_.size(); }

  bool empty() const { return index_map_.empty(); }

  bool contains(const Key &key) const {
    return index_map_.find(key) != index_map_.end();
  }

  const Value &operator[](const Key &key) {
    auto it = index_map_.find(key);
    if (it != index_map_.end()) {
      // Cache hit.
      IndexType index = it->second;
      Node &node = value_list_[index];
      if constexpr (ByAccessOrder) {
        // Update the last access order.
        value_list_.move_to_front(index);
      }
      return node.value();
    }
    // Fetch the value.
    Value &&new_value = value_provider_(key);
    if (max_size() == size()) {
      // Cache is full, drop the last entry and replace it.
      return replace_oldest_entry(key, std::move(new_value));
    }
    // Append at the back of the cache.
    IndexType new_index = size();
    const auto &value =
        value_list_.emplace_back(key, std::move(new_value), new_index);
    index_map_.emplace(key, new_index);
    return value;
  }

private:
  const Value &replace_oldest_entry(const Key &key, Value new_value) {
    Node &oldest_node = value_list_.last();
    index_map_.erase(oldest_node.key());
    dropped_entry_callback_(std::move(oldest_node.key()),
                            std::move(oldest_node.value()));
    Node &one_before_last = value_list_[oldest_node.prev_];
    IndexType oldest_node_index = one_before_last.next_;
    index_map_.emplace(key, oldest_node_index);

    oldest_node.key() = key;
    oldest_node.value() = std::move(new_value);

    oldest_node.next_ = value_list_.latest_;
    value_list_.first().prev_ = oldest_node_index;
    one_before_last.next_ = INVALID_INDEX;
    value_list_.oldest_ = oldest_node.prev_;
    oldest_node.prev_ = INVALID_INDEX;
    value_list_.latest_ = oldest_node_index;

    return oldest_node.value();
  }

  const IndexType max_size() const {
    return static_cast<const CRTPBase *>(this)->max_size();
  }

  struct Node {
    Node() = default;

    Node(Key key, Value value, IndexType prev, IndexType next)
        : prev_(prev), next_(next) {
      new (&this->value()) Value(std::move(value));
      new (&this->key()) Key(std::move(key));
    }

    ~Node() { value_pair().~value_type(); }

    Value &value() { return value_pair().second; }

    Key &key() { return value_pair().first; }

    const value_type &value_pair() const {
      return *reinterpret_cast<const value_type *>(&value_pair_);
    }

    value_type &value_pair() {
      return *reinterpret_cast<value_type *>(&value_pair_);
    }

    IndexType prev_;
    IndexType next_;

  private:
    // The content of the key and value. Initially uninitialized.
    typename std::aligned_storage<sizeof(value_type), alignof(value_type)>::type
        value_pair_;
  };

  struct LinkedList {
    Node &last() {
      assert(oldest_ != INVALID_INDEX);
      return list_content_[oldest_];
    }

    Node &first() {
      assert(latest_ != INVALID_INDEX);
      return list_content_[latest_];
    }

    Value &emplace_back(Key key, Value value, IndexType new_index) {
      if constexpr (has_emplace_back<Array<Node>, Node>) {
        list_content_.emplace_back(std::move(key), std::move(value),
                                   INVALID_INDEX, latest_);
      } else {
        list_content_[new_index] =
            Node{std::move(key), std::move(value), oldest_, INVALID_INDEX};
      }
      if (oldest_ == INVALID_INDEX) {
        oldest_ = new_index;
      } else {
        first().prev_ = new_index;
      }
      latest_ = new_index;
      return first().value();
    }

    void move_to_front(IndexType index) {
      if (index == latest_)
        return;
      Node &node = list_content_[index];
      // It's not the first, so it has a prev.
      assert(node.prev_ != INVALID_INDEX);
      Node &prev_node = list_content_[node.prev_];
      prev_node.next_ = node.next_;
      if (node.next_ != INVALID_INDEX) {
        list_content_[node.next_].prev_ = node.prev_;
      } else {
        oldest_ = node.prev_;
      }
      node.next_ = latest_;
      list_content_[latest_].prev_ = index;
      node.prev_ = INVALID_INDEX;
      latest_ = index;
    }

    Node &operator[](IndexType index) { return list_content_[index]; }

    const Node &operator[](IndexType index) const {
      return list_content_[index];
    }

    Array<Node> list_content_;
    IndexType latest_ = INVALID_INDEX;
    IndexType oldest_ = INVALID_INDEX;
  };

public:
  template <typename IteratorValueType, bool Reversed> class Iterator {
  public:
    using difference_type = std::ptrdiff_t;
    using value_type = IteratorValueType;
    using pointer = IteratorValueType *;
    using reference = IteratorValueType &;
    using iterator_category = std::bidirectional_iterator_tag;

    Iterator() = default;

    Iterator(const LinkedList &list, IndexType index)
        : linked_list_(&list), current_index_(index) {}

    IteratorValueType &operator*() { return node().value_pair(); }

    Iterator &operator++() {
      to_next();
      return *this;
    }

    Iterator operator++(int) {
      Iterator tmp = *this;
      ++*this;
      return tmp;
    }

    Iterator &operator--() {
      to_prev();
      return *this;
    }

    Iterator operator--(int) {
      Iterator tmp = *this;
      --*this;
      return tmp;
    }

    bool operator==(const Iterator &other) const {
      if (other.current_index_ == INVALID_INDEX &&
          current_index_ == INVALID_INDEX) {
        return true;
      }
      return other.linked_list_ == linked_list_ &&
             other.current_index_ == current_index_;
    }

    bool operator!=(const Iterator &other) const { return !(*this == other); }

  private:
    void to_next() {
      if constexpr (Reversed) {
        current_index_ = node().prev_;
      } else {
        current_index_ = node().next_;
      }
    }

    void to_prev() {
      if constexpr (Reversed) {
        current_index_ = node().next_;
      } else {
        current_index_ = node().prev_;
      }
    }

    const Node &node() const {
      assert(current_index_ != INVALID_INDEX);
      return (*linked_list_)[current_index_];
    }
    const LinkedList *linked_list_;
    IndexType current_index_;
  };

  using iterator = Iterator<value_type, false>;
  using const_iterator = Iterator<const value_type, false>;
  using reversed_iterator = Iterator<value_type, true>;
  using const_reversed_iterator = Iterator<const value_type, true>;

  iterator begin() {
    if (size() == 0)
      return {value_list_, INVALID_INDEX};
    return {value_list_, value_list_.latest_};
  }

  iterator end() { return {value_list_, INVALID_INDEX}; }

  const_iterator begin() const {
    if (size() == 0)
      return {value_list_, INVALID_INDEX};
    return {value_list_, value_list_.latest_};
  }

  const_iterator end() const { return {value_list_, INVALID_INDEX}; }

  reversed_iterator rbegin() {
    if (size() == 0)
      return {value_list_, INVALID_INDEX};
    return {value_list_, value_list_.oldest_};
  }

  reversed_iterator rend() { return {value_list_, INVALID_INDEX}; }

  const_reversed_iterator rbegin() const {
    if (size() == 0)
      return {value_list_, INVALID_INDEX};
    return {value_list_, value_list_.oldest_};
  }

  const_reversed_iterator rend() const { return {value_list_, INVALID_INDEX}; }

  template <typename K> iterator find(const K &key) {
    auto it = index_map_.find(key);
    if (it == index_map_.end()) {
      return end();
    }
    return {value_list_, it->second};
  }

  template <typename K> const_iterator find(const K &key) const {
    auto it = index_map_.find(key);
    if (it == index_map_.end()) {
      return end();
    }
    return {value_list_, it->second};
  }

private:
  Map<Key, IndexType> index_map_;
  LinkedList value_list_;
  ValueProvider value_provider_;
  DroppedEntryCallback dropped_entry_callback_;
};

} // namespace lru_cache::internal

#endif // LRU_CACHE_LRU_CACHE_IMPL_H_
