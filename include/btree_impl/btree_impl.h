#pragma once

namespace btree {
    template<typename K, typename V>
    BTree<K, V>::BTree(const int16_t order, IOManagerT& io) : t(order), root() {
//    pthread_rwlock_init(&(rwLock), NULL);
        if (!io.is_ready())
            return;

        auto root_pos = io.read_header();
        if (root_pos == IOManagerT::INVALID_ROOT_POS)
            return;

        root = io.read_node(root_pos);
    }

    template<typename K, typename V>
    void BTree<K, V>::set(IOManagerT& io, const K &key, const V &value) {
//    pthread_rwlock_wrlock(&(rwLock));

        EntryT e {key, value};
        if (!root.is_valid() || !root.set(io, e))
            insert(io, e);

//    pthread_rwlock_unlock(&(rwLock));
    }

    template<typename K, typename V>
    void BTree<K, V>::set(IOManagerT& io, const K &key, const V& value, int32_t size) {
//    pthread_rwlock_wrlock(&(rwLock));

        EntryT e {key, value, size};
        if (!root.is_valid() || !root.set(io, e))
            insert(io, e);

//    pthread_rwlock_unlock(&(rwLock));
    }

    template<typename K, typename V>
    std::optional <V> BTree<K, V>::get(IOManagerT& io, const K &key) {
        EntryT res = root.is_valid() ? root.find(io, key) : EntryT{};
        return res.value();
    }

    template<typename K, typename V>
    bool BTree<K, V>::exist(IOManagerT& io, const K &key) {
//    pthread_rwlock_wrlock(&(rwLock));

        bool success = root.is_valid() && root.find(io, key).is_valid();

//    pthread_rwlock_unlock(&(rwLock));
        return success;
    }

    template<typename K, typename V>
    bool BTree<K, V>::remove(IOManagerT& io, const K &key) {
//    pthread_rwlock_wrlock(&(rwLock));

        bool success = root.is_valid() && root.remove(io, key);

        if (success && root.used_keys == 0) {
            if (root.is_leaf) {
                root = Node();
                io.write_invalidated_root();
            } else {
                auto pos = root.child_pos[0];
                io.write_new_pos_for_root_node(pos);
                root = io.read_node(pos);
            }
        }

//    pthread_rwlock_unlock(&(rwLock));
        return success;
    }

    template<typename K, typename V>
    void BTree<K, V>::insert(IOManagerT& io, const EntryT& e) {
        if (!root.is_valid()) {
            // write header
            auto root_pos = io.write_header();

            root = Node(t, true);
            root.m_pos = root_pos;
            root.used_keys++;

            auto entry_pos = root.m_pos + root.get_node_size_in_bytes();
            root.key_pos[0] = entry_pos;

            // write node root and key|value
            io.write_node(root, root.m_pos);
            io.write_entry(e, entry_pos);
        } else {
            if (root.is_full()) {
                Node newRoot(t, false);
                newRoot.child_pos[0] = root.m_pos;

                // Write node
                newRoot.m_pos = io.get_file_pos_end();
                io.write_node(newRoot, newRoot.m_pos);

                newRoot.split_child(io, 0, root);

                // Find the child have new key
                int32_t i = 0;
                K root_key = newRoot.get_key(io, 0);
                if (root_key < e.key)
                    i++;

                // Read node
                auto pos = newRoot.child_pos[i];
                Node node = io.read_node(pos);
                node.insert_non_full(io, e);

                root = io.read_node(newRoot.m_pos);
                io.write_new_pos_for_root_node(newRoot.m_pos);
            } else {
                root.insert_non_full(io, e);
            }
        }
    }
}