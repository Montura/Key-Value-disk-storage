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
    char flag;
//    int lenKey, lenValue;

    file.setPosFile(pos);

//    readDisk->readByte((uint8_t&) flag);
//    readDisk->readBytes(reinterpret_cast<uint8_t*> (&lenKey), 0, sizeof (lenKey));
//    key.resize(lenKey);
//    readDisk->readBytes((uint8_t*) &key, 0, 4);
//    readDisk->readBytes(reinterpret_cast<uint8_t*> (&lenValue), 0, sizeof (lenValue));
//    value.resize(lenValue);
//    readDisk->readBytes((uint8_t*) &value, 0, 4);

    flag = file.read_byte();
    entry.key = file.read_next<K>();
    entry.value = file.read_next<V>();
    assert(flag > -1);
    assert(entry.key > -1);
}

template<class K, class V>
void BTree<K, V>::write_flag(char flag, const int pos) {
    file.setPosFile(pos);

    file.write_byte(flag);
}

template<class K, class V>
int BTree<K, V>::calc_node_writable_node_size() {
    return
        sizeof (int) * root->max_key_num() +   // key positions array
        sizeof (int) * root->max_child_num() + // child positions array
        /* flag: is_deleted and is_leaf */ 1 +
        /* sizeof(used_keys) */ 4;
}

template<class K, class V>
void BTree<K, V>::insert(const EntryT& entry) {
    if (root == NULL) {
        root = new Node(t, true);
        write_header(t, 8);
        root->m_pos = 8;

        file.setPosEndFile();
        
        int pos = file.getPosFile();
        pos += calc_node_writable_node_size();

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

            int posFile = file.getPosFile();
            newRoot.m_pos = posFile;
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
const V BTree<K, V>::get(const K& key) {
//    pthread_rwlock_wrlock(&(this->rwLock));

    if (!root) {
//        pthread_rwlock_unlock(&(this->rwLock));
        return -1;
    }

    auto entry = root->find(this, key);
    if (entry.key == Entry<K,V>::INVALID_KEY) {
//        pthread_rwlock_unlock(&(this->rwLock));
        return -1;
    }

//    pthread_rwlock_unlock(&(this->rwLock));
    return entry.value;
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

    bool success = root->find(this, key).key != Entry<K,V>::INVALID_KEY;
//    timeFinish = get_timestamp();
//    secs = (timeFinish - timeStart);

//    cout << "time API exist: " << secs << " microsecond" << endl;

//    pthread_rwlock_unlock(&(rwLock));
    return success;
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
    if (root != NULL) {
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

