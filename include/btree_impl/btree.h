#pragma once

#include "utils/forward_decl.h"
#include "btree_node.h"

namespace btree {

    template <typename K, typename V>
    struct BTree final {
        using Node = BTreeNode<K,V>;
        using EntryT = Entry<K, V>;
        using IOManagerT = IOManager<K, V>;

        BTree(const int16_t order, IOManagerT& io);

        bool exist(IOManagerT& io, const K &key);
        void set(IOManagerT& io, const K &key, const V &value);
        void set(IOManagerT& io, const K &key, const V& value, const int32_t size);

        std::optional<V> get(IOManagerT& io, const K &key);
        bool remove(IOManagerT& io, const K &key);

    private:
        void insert(IOManagerT& io, const EntryT& e);

        const int16_t t;
        BTreeNode<K,V> root;
    };
}

#include "btree_impl/btree_impl.h"

