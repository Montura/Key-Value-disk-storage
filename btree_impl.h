#pragma once

#include "btree.h"
#include "io_manager_impl.h"

template <typename K, typename V>
BTree<K, V>::BTree(const std::string& path, int order) : t(order), io_manager(path) {
//    pthread_rwlock_init(&(rwLock), NULL);
    if (!io_manager.is_ready())
        return;

    int t_from_file = 0;
    int root_pos = io_manager.read_header(t_from_file);
    assert(t == t_from_file);

    if (root_pos == IOManager<K,V>::INVALID_ROOT_POS)
        return;

    root = new Node(t, false);
    io_manager.read_node(root, root_pos);
}

template <typename K, typename V>
BTree<K, V>::~BTree() {
    delete root;
//    pthread_rwlock_destroy(&(rwLock));
}

template <typename K, typename V>
void BTree<K, V>::insert(const K& key, const V& value) {
    if (root == nullptr) {
        // write header
        int root_pos = io_manager.write_header(t);

        root = new Node(t, true);
        root->m_pos = root_pos;
        root->used_keys++;

        // write node root and key|value
        int pos = io_manager.write_node(*root, root->m_pos);
        io_manager.write_entry({ key, value }, pos);

        root->key_pos[0] = pos;
    } else {
        if (root->is_full()) {
            Node newRoot(t, false);
            newRoot.child_pos[0] = root->m_pos;

            // Write node
            int posFile = io_manager.get_file_pos_end();
            newRoot.m_pos = posFile;
            io_manager.write_node(newRoot, newRoot.m_pos);

            newRoot.split_child(io_manager, 0, *root);

            // Find the children have new key
            int i = 0;
            K root_key = newRoot.read_key(io_manager, 0);
            if (root_key < key)
                i++;

            //read node
            Node node(t, false);
            int pos = newRoot.child_pos[i];
            io_manager.read_node(&node, pos);
            node.insert_non_full(io_manager, key, value);

            io_manager.read_node(root, newRoot.m_pos);
            io_manager.writeUpdatePosRoot(newRoot.m_pos);
        } else {
            root->insert_non_full(io_manager, key, value);
        }
    }
}

template <typename K, typename V>
const V BTree<K, V>::get(const K& key) {
//    pthread_rwlock_wrlock(&(this->rwLock));

    auto entry = root ? root->find(io_manager, key) : EntryT { EntryT::INVALID_KEY, V() };

//    pthread_rwlock_unlock(&(this->rwLock));
    return entry.value;
}

template <typename K, typename V>
void BTree<K, V>::set(const K& key, const V& value) {
//    pthread_rwlock_wrlock(&(rwLock));
    //    int secs;
    //    timestamp_t timeFinish;
    //    timestamp_t timeStart = get_timestamp();

    if (!root || !root->set(io_manager, key, value))
        insert(key, value);
        //        timeFinish = get_timestamp();


    //    timeFinish = get_timestamp();
    //    secs = (timeFinish - timeStart);
    //
    //    cout << " time API set: " << secs << " microsecond" << endl;

//    pthread_rwlock_unlock(&(rwLock));
}

template <typename K, typename V>
bool BTree<K, V>::exist(const K& key) {
//    pthread_rwlock_wrlock(&(rwLock));

//    int secs;
//    timestamp_t timeFinish;
//    timestamp_t timeStart = get_timestamp();

    bool success = root && root->find(io_manager, key).key != Entry<K,V>::INVALID_KEY;
//    timeFinish = get_timestamp();
//    secs = (timeFinish - timeStart);

//    cout << "time API exist: " << secs << " microsecond" << endl;

//    pthread_rwlock_unlock(&(rwLock));
    return success;
}

template <typename K, typename V>
bool BTree<K, V>::remove(const K& key) {
//    pthread_rwlock_wrlock(&(rwLock));
//    int secs;
//    timestamp_t timeFinish;
//    timestamp_t timeStart = get_timestamp();

    bool success = root && root->remove(io_manager, key);

    if (success && root->used_keys == 0) {
        if (root->is_leaf()) {
            io_manager.write_flag(root->is_deleted_or_is_leaf(), root->m_pos);
            delete root;
            root = nullptr;
            io_manager.writeUpdatePosRoot(IOManager<K,V>::INVALID_ROOT_POS);
        } else {
            int pos = root->child_pos[0];
            io_manager.writeUpdatePosRoot(pos);
            io_manager.read_node(root, pos);
        }
    }

//    timeFinish = get_timestamp();
//    secs = (timeFinish - timeStart);

//    cout << "time API remove : " << secs << " microsecond" << endl;

//    pthread_rwlock_unlock(&(rwLock));
    return success;
}

template <typename K, typename V>
void BTree<K, V>::traverse() {
    if (root != nullptr)
        root->traverse(io_manager);
}
