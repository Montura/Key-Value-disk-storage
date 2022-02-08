#pragma once

#include <type_traits>
#include <cstdint>
#include <optional>

#include "entry.h"
#include "btree_node.h"
#include "utils/forward_decl.h"

namespace btree {
    template <typename K, typename V>
    struct BTree final {
        static constexpr bool is_valid_blob = std::is_pointer_v<V> && std::is_same_v<std::remove_pointer_t<V>, const char>;
        static_assert(std::is_arithmetic_v<V> || is_string_v<V> || is_valid_blob);

        using ValueType = conditional_t<std::is_arithmetic_v<V>, const V, const V&>;

        using EntryT = entry::Entry<K,V>;
        using Node = BTreeNode<K, V>;
        using IOManagerT = IOManager<K, V>;

        BTree(const int16_t order, IOManagerT& io);

        bool exist(IOManagerT& io, const K key) const;
        std::optional<V> get(IOManagerT& io, const K key) const;
        void set(IOManagerT& io, const K key, ValueType value);
        void set(IOManagerT& io, const K key, const V& value, const int32_t size);
        bool remove(IOManagerT& io, const K key);
    private:
        void insert(IOManagerT& io, const EntryT& e);

        const int16_t t;
        BTreeNode<K, V> root;
    };
}

#include "btree_impl/btree_impl.h"

