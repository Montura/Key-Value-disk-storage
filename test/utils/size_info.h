#pragma once

#include "btree_impl/btree_node.h"
#include "io/io_manager.h"
#include "utils/utils.h"

namespace tests {
    using namespace utils;

    template <typename K, typename V>
    struct SizeInfo {
        static constexpr int32_t header_size_in_bytes() {
            return btree::IOManager<K,V>::INITIAL_ROOT_POS_IN_HEADER;
        }

        static constexpr int32_t file_size_in_bytes(const int t, const K key, const V& val, const int32_t value_size) {
            return header_size_in_bytes() + node_size(t) + full_entry_size_in_bytes(key, val, value_size);
        }

    private:
        static constexpr int32_t node_size(const int t) {
            return btree::BTreeNode<K,V>::get_node_size_in_bytes(t);
        }

        static constexpr int32_t full_entry_size_in_bytes(const K key, const V& val, const int32_t value_size) {
            int32_t key_size = 4 + key.size(); // std::string
            if constexpr (std::is_pointer_v<V> || is_string_v<V>) {
                int32_t value_len = 4; // sizeof(int32_t) for storing the length of value
                return key_size + value_len + value_size;
            } else {
                return key_size + value_size;
            }
        }
    };
}