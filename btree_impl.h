#pragma once

#include "btree.h"
#include "btree_node.h"

template<class K, class V>
BTreeStore<K, V> ::BTreeStore(const std::string& path, int order) : t(order) {
    file = new MappedFile(path, 0);
//    pthread_rwlock_init(&(rwLock), NULL);

    if (file->isEmpty()) {
        root = NULL;
        return;
    }
    int root_pos;
    int t_2 = 0;
    readHeader(t_2, root_pos);
    assert(t == t_2);

    if (root_pos == -1) {
        root = NULL;
        return;
    }

    root = new BTreeNodeStore<K, V>(t, false);
    readNode(root, root_pos);
}

template<class K, class V>
BTreeStore<K, V> ::~BTreeStore() {
    delete root;
    delete file;

//    pthread_rwlock_destroy(&(rwLock));
}

template<class K, class V>
void BTreeStore<K, V>::readHeader(int& t, int& posRoot) {
    file->setPosFile(0);
    t = file->read_int();
    posRoot = file->read_int();
}

template<class K, class V>
void BTreeStore<K, V> ::writeHeader(const int t, const int posRoot) {
    file->setPosFile(0);

    file->write_int(t);
    file->write_int(posRoot);
}

template<class K, class V>
void BTreeStore<K, V> ::writeUpdatePosRoot(const int posRoot) {
    file->setPosFile(4);

    file->write_int(posRoot);
}

template<class K, class V>
void BTreeStore<K, V> ::writeNode(BTreeNodeStore<K, V>& node, const int pos) {
    file->setPosFile(pos);

    file->write_byte(node.flag);
    file->write_int(node.nCurrentEntry);
    file->write_vector(node.arrayPosKey);
    file->write_vector(node.arrayPosChild);
}

template<class K, class V>
void BTreeStore<K, V>::readNode(BTreeNodeStore<K, V>* node, const int pos) {
    file->setPosFile(pos);

    node->m_pos = pos;
    node->flag = file->read_byte();
    node->nCurrentEntry = file->read_int();
    file->read_vector(node->arrayPosKey);
    file->read_vector(node->arrayPosChild);
}

template<class K, class V>
void BTreeStore<K, V>::writeEntry(const Entry<K, V>& entry, const int pos) {
    char flag = 1;
    int strKey = entry.key;
    int strValue = entry.value;

    file->setPosFile(pos);

    file->write_byte(flag);
    file->write_next(strKey);
    file->write_next(strValue);
}

template<class K, class V>
void BTreeStore<K, V>::readEntry(Entry<K, V>& entry, const int pos) {
    int key;
    int value;
    char flag;
//    int lenKey, lenValue;

    file->setPosFile(pos);

//    readDisk->readByte((uint8_t&) flag);
//    readDisk->readBytes(reinterpret_cast<uint8_t*> (&lenKey), 0, sizeof (lenKey));
//    key.resize(lenKey);
//    readDisk->readBytes((uint8_t*) &key, 0, 4);
//    readDisk->readBytes(reinterpret_cast<uint8_t*> (&lenValue), 0, sizeof (lenValue));
//    value.resize(lenValue);
//    readDisk->readBytes((uint8_t*) &value, 0, 4);

    
    flag = file->read_byte();
    key = file->read_next<K>();
    value = file->read_next<V>();
    assert(flag > -1);
    assert(key > -1);
//    assert(value >= 65);

    entry.setKeyValue(key, value);
}

template<class K, class V>
void BTreeStore<K, V> ::writeFlag(char flag, const int pos) {
    file->setPosFile(pos);

    file->write_byte(flag);
}

template<class K, class V>
void BTreeStore<K, V> ::insert(const Entry<K, V>& entry) {
    if (root == NULL) {
        root = new BTreeNodeStore<K, V>(t, true);
        writeHeader(t, 8);
        root->m_pos = 8;

        file->setPosEndFile();
        
        int pos = file->getPosFile();
        pos = pos + sizeof (int) * (2 * t - 1) + sizeof (int) * (2 * t) + 5; // 1 byte flag + 4 byte nCurrentKey

        root->arrayPosKey[0] = pos;
        root->flag = 1;
        root->nCurrentEntry++;

        //write node root
        writeNode(*root, root->m_pos);

        //write key value
        writeEntry(entry, pos);
    } else {
        if (root->nCurrentEntry == 2 * t - 1) {
            BTreeNodeStore<K, V>* newRoot = new BTreeNodeStore<K, V>(t, false);

            newRoot->arrayPosChild[0] = root->m_pos;

            file->setPosEndFile();

            int posFile = file->getPosFile();
            newRoot->m_pos = posFile;
            //write node
            writeNode(*newRoot, newRoot->m_pos);

            newRoot->splitChild(this, 0, *root);
            //find child have new key
            int i = 0;
            Entry<K, V> entryOfRoot = newRoot->getEntry(this, 0).value();
            if (entryOfRoot.key < entry.key) {
                i++;
            }

            BTreeNodeStore<K, V>* node = new BTreeNodeStore<K, V>(t, false);
            int pos = newRoot->arrayPosChild[i];

            //read node
            readNode(node, pos);

            node->insertNotFull(this, entry);

            readNode(root, newRoot->m_pos);

            //cap nhat lai header
            writeUpdatePosRoot(newRoot->m_pos);

            delete newRoot;

            delete node;
        } else {
            root->insertNotFull(this, entry);
        }
    }
}

template<class K, class V>
const V BTreeStore<K, V>::getValue(const K& key) {
//    pthread_rwlock_wrlock(&(this->rwLock));

    if (root == NULL) {
//        pthread_rwlock_unlock(&(this->rwLock));
        return -1;
    }

    auto opt = root->search(this, key);

    if (!opt.has_value()) {
//        pthread_rwlock_unlock(&(this->rwLock));
        return -1;
    }
    auto value = opt.value().value;

//    pthread_rwlock_unlock(&(this->rwLock));
    return value;
}

template<class K, class V>
void BTreeStore<K, V> ::set(const K& key, const V& value) {
//    pthread_rwlock_wrlock(&(rwLock));
    //    int secs;
    //    timestamp_t timeFinish;
    //    timestamp_t timeStart = get_timestamp();

    if (root == NULL) {
        Entry<K, V> entry(key, value);
        insert(entry);
    } else if (!root->set(this, key, value)) {
        Entry<K, V> entry(key, value);
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
bool BTreeStore<K, V> ::exist(const K& key) {
//    pthread_rwlock_wrlock(&(rwLock));

//    int secs;
//    timestamp_t timeFinish;
//    timestamp_t timeStart = get_timestamp();

    if (root == NULL) {
//        timeFinish = get_timestamp();
//        secs = (timeFinish - timeStart);

//        cout << "time API exist: " << secs << " microsecond" << endl;
//        pthread_rwlock_unlock(&(rwLock));
        return false;
    }

    bool res;

    auto opt = root->search(this, key);

    res = opt.has_value();

//    timeFinish = get_timestamp();
//    secs = (timeFinish - timeStart);

//    cout << "time API exist: " << secs << " microsecond" << endl;

//    pthread_rwlock_unlock(&(rwLock));
    return res;
}

template<class K, class V>
bool BTreeStore<K, V> ::remove(const K& key) {
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

    if (root->nCurrentEntry == 0) {
        if (root->checkIsLeaf()) {
            char flag = root->flag;
            flag = flag | (1 << 1);
            writeFlag(flag, root->m_pos);
            delete root;
            root = NULL;
            writeUpdatePosRoot(-1);
        } else {
            int pos = root->arrayPosChild[0];
            writeUpdatePosRoot(pos);
            readNode(root, pos);
        }
    }

//    timeFinish = get_timestamp();
//    secs = (timeFinish - timeStart);

//    cout << "time API remove : " << secs << " microsecond" << endl;

//    pthread_rwlock_unlock(&(rwLock));
    return res;
}

template<class K, class V>
void BTreeStore<K, V> ::traverse() {
    if (root != NULL) {
        root->traverse(this);
    }
}

template<class K, class V>
int BTreeStore<K, V> ::getPosFileWrite() const {
    return file->getPosFile();
}

template<class K, class V>
void BTreeStore<K, V>::setPosEndFileWrite() {
    file->setPosEndFile();
}

