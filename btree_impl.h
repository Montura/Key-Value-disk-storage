#pragma once

#include "btree.h"
#include "io_manager_impl.h"

template <typename K, typename V>
BTree<K, V>::BTree(const std::string& path, int order) : t(order), io_manager(path) {
//    pthread_rwlock_init(&(rwLock), NULL);

    if (!io_manager.is_ready()) {
        root = nullptr;
        return;
    }
    int root_pos;
    int t_2 = 0;
    io_manager.read_header(t_2, root_pos);
    assert(t == t_2);

    if (root_pos == -1) {
        root = nullptr;
        return;
    }

    root = new Node(t, false);
    io_manager.read_node(root, root_pos);
}

template <typename K, typename V>
BTree<K, V>::~BTree() {
    delete root;

//    pthread_rwlock_destroy(&(rwLock));
}


template <typename K, typename V>
void BTree<K, V>::insert(const EntryT& entry) {
    if (root == nullptr) {
        root = new Node(t, true);
        io_manager.write_header(t, 8);
        root->m_pos = 8;

        io_manager.setPosEndFile();

        int pos = io_manager.getPosFile();
        pos += sizeof (int) * (2 * t - 1) + sizeof (int) * (2 * t) + 5; // 1 byte flag + 4 byte nCurrentKey

        root->arrayPosKey[0] = pos;
        root->used_keys++;

        // write node root
        io_manager.write_node(*root, root->m_pos);

        //write key value
        io_manager.write_entry(entry, pos);
    } else {
        if (root->is_full()) {
            Node newRoot(t, false);

            newRoot.arrayPosChild[0] = root->m_pos;

            io_manager.setPosEndFile();

            newRoot.m_pos = io_manager.getPosFile();
            //write node
            io_manager.write_node(newRoot, newRoot.m_pos);

            newRoot.split_child(io_manager, 0, *root);
            //find child have new key
            int i = 0;
            EntryT entryOfRoot = newRoot.read_entry(io_manager, 0);
            if (entryOfRoot.key < entry.key) {
                i++;
            }

            Node node(t, false);
            int pos = newRoot.arrayPosChild[i];

            //read node
            io_manager.read_node(&node, pos);

            node.insert_non_full(io_manager, entry);

            io_manager.read_node(root, newRoot.m_pos);

            io_manager.writeUpdatePosRoot(newRoot.m_pos);
        } else {
            root->insert_non_full(io_manager, entry);
        }
    }
}

template <typename K, typename V>
void BTree<K, V>::set(const K& key, const V& value) {
//    pthread_rwlock_wrlock(&(rwLock));
    //    int secs;
    //    timestamp_t timeFinish;
    //    timestamp_t timeStart = get_timestamp();

    if (!root) {
        EntryT entry { key, value };
        insert(entry);
    } else if (!root->set(io_manager, key, value)) {
        EntryT entry { key, value };
        insert(entry);
        //        timeFinish = get_timestamp();
    }

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

    if (!root) {
//        timeFinish = get_timestamp();
//        secs = (timeFinish - timeStart);

//        cout << "time API exist: " << secs << " microsecond" << endl;
//        pthread_rwlock_unlock(&(rwLock));
        return false;
    }

    bool res;

    Entry<K, V> entry = root->find(io_manager, key);
    res = !entry.is_dummy();

//    timeFinish = get_timestamp();
//    secs = (timeFinish - timeStart);

//    cout << "time API exist: " << secs << " microsecond" << endl;

//    pthread_rwlock_unlock(&(rwLock));
    return res;
}

template <typename K, typename V>
bool BTree<K, V>::remove(const K& key) {
//    pthread_rwlock_wrlock(&(rwLock));
//    int secs;
//    timestamp_t timeFinish;
//    timestamp_t timeStart = get_timestamp();

    if (!root) {
//        timeFinish = get_timestamp();
//        secs = (timeFinish - timeStart);

//        cout << "time API remove : " << secs << " microsecond" << endl;

//        pthread_rwlock_unlock(&(rwLock));
        return false;
    }

    bool res = root->remove(io_manager, key);

    if (root->used_keys == 0) {
        if (root->is_leaf()) {
            char flag = root->flag;
            flag = flag | (1 << 1);
            io_manager.write_flag(flag, root->m_pos);
            delete root;
            root = nullptr;
            io_manager.writeUpdatePosRoot(-1);
        } else {
            int pos = root->arrayPosChild[0];
            io_manager.writeUpdatePosRoot(pos);
            io_manager.read_node(root, pos);
        }
    }

//    timeFinish = get_timestamp();
//    secs = (timeFinish - timeStart);

//    cout << "time API remove : " << secs << " microsecond" << endl;

//    pthread_rwlock_unlock(&(rwLock));
    return res;
}

template <typename K, typename V>
void BTree<K, V>::traverse() {
    if (!root) {
        root->traverse(io_manager);
    }
}
