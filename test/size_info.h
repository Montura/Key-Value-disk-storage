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

        static constexpr int32_t file_size_in_bytes(const int t, const K& key, const V& val, const bool after_remove) {
            return header_size_in_bytes() + (after_remove ? 0 : node_size(t) + entry_size_in_bytes(key, val));
        }

        static constexpr int32_t value_size_in_bytes(const V& value) {
            if constexpr (std::is_pointer_v<V>) {
                static_assert(sizeof(std::remove_pointer_t<V>) == 1);
                return static_cast<int32_t>(std::strlen(value));
            } else {
                if constexpr (is_string_v<V>) {
                    return static_cast<int32_t>(value.size() * sizeof(typename V::value_type));
                } else {
                    return sizeof(V);
                }
            }
        }

    private:
        static constexpr int32_t node_size(const int t) {
            return btree::BTreeNode<K,V>::get_node_size_in_bytes(t);
        }

        static constexpr int32_t entry_size_in_bytes(const K& key, const V& val) {
            int32_t key_size = sizeof(K);
            if constexpr (std::is_pointer_v<V>) {
                int32_t value_len = 4; // size
                return key_size + value_len + value_size_in_bytes(val);
            } else {
                if constexpr (is_string_v<V>) {
                    int32_t value_len = 4; // size
                    return key_size + value_len + value_size_in_bytes(val);
                } else {
                    return key_size + value_size_in_bytes(val);
                }
            }
        }
    };
}