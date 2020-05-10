#ifndef LRU_CACHE_ARRAY_NODE_CONTAINER_H_
#define LRU_CACHE_ARRAY_NODE_CONTAINER_H_

#include <array>

namespace lru_cache {

// A node container based on a static array of size N, similarly to std::array.
// The memory size is fixed, there is no insertion/deletion.
template <typename Node, size_t N, typename index_type = typename Node::IndexType> struct ArrayNodeContainer {
  using node_type = Node;
  using key_type = typename node_type::key_type;
  using IndexType = index_type;
  static constexpr IndexType INVALID_INDEX =
      std::numeric_limits<IndexType>::max();

  IndexType emplace_back(node_type node) {
    list_content_[size_] = std::move(node);
    ++size_;
    return size_ - 1;
  }

  // Only destroy the ones that were initialized.
  ~ArrayNodeContainer() {
    for (size_t i = 0; i < size_; ++i) {
      list_content_[i].~node_type();
    }
  }

  IndexType replace_entry(IndexType index, const key_type &old_key,
                          node_type new_node) {
    list_content_[index] = std::move(new_node);
    return index;
  }

  node_type &operator[](IndexType index) { return list_content_[index]; }
  const node_type &operator[](IndexType index) const {
    return list_content_[index];
  }

private:
  // We need to keep track of the size to destroy only the elements that were
  // constructed.
  size_t size_ = 0;
  node_type list_content_[N];
};

} // lru_cache

#endif // LRU_CACHE_ARRAY_NODE_CONTAINER_H_
