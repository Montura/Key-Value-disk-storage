#pragma once

#include "btree.h"

template<class K, class V>
BTree<K, V>::BTree(const std::string& path, int order) : t(order), file(path, 0) {
//    pthread_rwlock_init(&(rwLock), NULL);

    if (file.isEmpty()) {
        root = nullptr;
        return;
    }
    int root_pos;
    int t_2 = 0;
    read_header(t_2, root_pos);
    assert(t == t_2);

    if (root_pos == -1) {
        root = nullptr;
        return;
    }

    root = new Node(t, false);
    read_node(root, root_pos);
}

template<class K, class V>
BTree<K, V>::~BTree() {
    delete root;

//    pthread_rwlock_destroy(&(rwLock));
}

template<class K, class V>
void BTree<K, V>::read_header(int& t, int& posRoot) {
    file.setPosFile(0);
    t = file.read_int();
    posRoot = file.read_int();
}

template<class K, class V>
void BTree<K, V>::write_header(const int t, const int posRoot) {
    file.setPosFile(0);

    file.write_int(t);
    file.write_int(posRoot);
}

template<class K, class V>
void BTree<K, V>::writeUpdatePosRoot(const int posRoot) {
    file.setPosFile(4);

    file.write_int(posRoot);
}

template<class K, class V>
void BTree<K, V>::write_node(const Node& node, const int pos) {
    file.setPosFile(pos);

    file.write_byte(node.flag);
    file.write_int(node.used_keys);
    file.write_vector(node.arrayPosKey);
    file.write_vector(node.arrayPosChild);
}

template<class K, class V>
void BTree<K, V>::read_node(Node* node, const int pos) {
    file.setPosFile(pos);

    node->m_pos = pos;
    node->flag = file.read_byte();
    node->used_keys = file.read_int();
    file.read_vector(node->arrayPosKey);
    file.read_vector(node->arrayPosChild);
}

template<class K, class V>
void BTree<K, V>::write_entry(const EntryT& entry, const int pos) {
    char flag = 1;
    K strKey = entry.key;
    V strValue = entry.value;

    file.setPosFile(pos);

    file.write_byte(flag);
    file.write_next(strKey);
    file.write_next(strValue);
}

template<class K, class V>
void BTree<K, V>::read_entry(EntryT& entry, const int pos) {
//    int lenKey, lenValue;

    file.setPosFile(pos);

    char flag = file.read_byte();
    entry.key = file.read_next<K>();
    entry.value = file.read_next<V>();
}

template<class K, class V>
void BTree<K, V>::write_flag(char flag, const int pos) {
    file.setPosFile(pos);

    file.write_byte(flag);
}

template<class K, class V>
void BTree<K, V> ::insert(const EntryT& entry) {
    if (root == nullptr) {
        root = new Node(t, true);
        write_header(t, 8);
        root->m_pos = 8;

        file.setPosEndFile();

        int pos = file.getPosFile();
        pos += sizeof (int) * (2 * t - 1) + sizeof (int) * (2 * t) + 5; // 1 byte flag + 4 byte nCurrentKey

        root->arrayPosKey[0] = pos;
        root->used_keys++;

        // write node root
        write_node(*root, root->m_pos);

        //write key value
        write_entry(entry, pos);
    } else {
        if (root->is_full()) {
            Node newRoot(t, false);

            newRoot.arrayPosChild[0] = root->m_pos;

            file.setPosEndFile();

            newRoot.m_pos = file.getPosFile();
            //write node
            write_node(newRoot, newRoot.m_pos);

            newRoot.split_child(this, 0, *root);
            //find child have new key
            int i = 0;
            EntryT entryOfRoot = newRoot.read_entry(this, 0);
            if (entryOfRoot.key < entry.key) {
                i++;
            }

            Node node(t, false);
            int pos = newRoot.arrayPosChild[i];

            //read node
            read_node(&node, pos);

            node.insert_non_full(this, entry);

            read_node(root, newRoot.m_pos);

            writeUpdatePosRoot(newRoot.m_pos);
        } else {
            root->insert_non_full(this, entry);
        }
    }
}

template<class K, class V>
void BTree<K, V>::set(const K& key, const V& value) {
//    pthread_rwlock_wrlock(&(rwLock));
    //    int secs;
    //    timestamp_t timeFinish;
    //    timestamp_t timeStart = get_timestamp();

    if (!root) {
        EntryT entry { key, value };
        insert(entry);
    } else if (!root->set(this, key, value)) {
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

template<class K, class V>
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

    Entry<K, V> entry = root->find(this, key);
    res = !entry.is_dummy();

//    timeFinish = get_timestamp();
//    secs = (timeFinish - timeStart);

//    cout << "time API exist: " << secs << " microsecond" << endl;

//    pthread_rwlock_unlock(&(rwLock));
    return res;
}

template<class K, class V>
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

    bool res = root->remove(this, key);

    if (root->used_keys == 0) {
        if (root->is_leaf()) {
            char flag = root->flag;
            flag = flag | (1 << 1);
            write_flag(flag, root->m_pos);
            delete root;
            root = nullptr;
            writeUpdatePosRoot(-1);
        } else {
            int pos = root->arrayPosChild[0];
            writeUpdatePosRoot(pos);
            read_node(root, pos);
        }
    }

//    timeFinish = get_timestamp();
//    secs = (timeFinish - timeStart);

//    cout << "time API remove : " << secs << " microsecond" << endl;

//    pthread_rwlock_unlock(&(rwLock));
    return res;
}

template<class K, class V>
void BTree<K, V>::traverse() {
    if (!root) {
        root->traverse(this);
    }
}

template<class K, class V>
int BTree<K, V>::getPosFileWrite() {
    return file.getPosFile();
}

template<class K, class V>
void BTree<K, V>::setPosEndFileWrite() {
    file.setPosEndFile();
}

