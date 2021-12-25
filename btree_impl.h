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
int BTree<K, V>::calc_node_writable_node_size() {
    return
        sizeof (int) * root->max_key_num() +   // key positions array
        sizeof (int) * root->max_child_num() + // child positions array
        /* flag: is_deleted and is_leaf */ 1 +
        /* sizeof(used_keys) */ 4;
}

template <typename K, typename V>
void BTree<K, V>::insert(const EntryT& entry) {
    if (root == NULL) {
        root = new Node(t, true);
        io_manager.write_header(t, 8);
        root->m_pos = 8;

        io_manager.setPosEndFile();
        
        int pos = io_manager.getPosFile();
        pos += calc_node_writable_node_size();

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

            int posFile = io_manager.getPosFile();
            newRoot.m_pos = posFile;
            //write node
            io_manager.write_node(newRoot, newRoot.m_pos);

            newRoot.split_child(io_manager, 0, *root);
            // find the children have new key
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
const V BTree<K, V>::get(const K& key) {
//    pthread_rwlock_wrlock(&(this->rwLock));

    if (!root) {
//        pthread_rwlock_unlock(&(this->rwLock));
        return -1;
    }

    auto entry = root->find(io_manager, key);
    if (entry.key == Entry<K,V>::INVALID_KEY) {
//        pthread_rwlock_unlock(&(this->rwLock));
        return -1;
    }

//    pthread_rwlock_unlock(&(this->rwLock));
    return entry.value;
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

    bool success = root->find(io_manager, key).key != Entry<K,V>::INVALID_KEY;
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
    return success;
}

template <typename K, typename V>
void BTree<K, V>::traverse() {
    if (root != NULL) {
        root->traverse(io_manager);
    }
}
