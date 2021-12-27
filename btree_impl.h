#pragma once

#include "btree.h"
#include "btree_node.h"

template<class K, class V>
BTreeStore<K, V> ::BTreeStore(const std::string& path, int order) : PATH(path), t(order) {
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
void BTreeStore<K, V> ::writeNode(BTreeNodeStore<K, V>* node, const int pos) {
    file->setPosFile(pos);

    int* arrPosKey = node->arrayPosKey;
    int* arrPosChild = node->arrayPosChild;
    char flag = node->flag;
    int nCurrentEntry = node->nCurrentEntry;
    int size = 2 * t;

    file->write_byte(flag);
    file->write_int(nCurrentEntry);
    file->write_int_array(arrPosKey, size - 1);
    file->write_int_array(arrPosChild, size);
}

template<class K, class V>
void BTreeStore<K, V>::readNode(BTreeNodeStore<K, V>* node, const int pos) {
    int size = 2 * t;

    file->setPosFile(pos);

    node->flag = file->read_byte();
    node->m_pos = pos;
    node->t = t;
    node->nCurrentEntry = file->read_int();
    file->read_int_array(node->arrayPosKey, size - 1);
    file->read_int_array(node->arrayPosChild, size);
}

template<class K, class V>
void BTreeStore<K, V>::writeEntry(const Entry<K, V>* entry, const int& pos) {
    char flag = 1;
    int strKey = entry->getKey();
    int strValue = entry->getValue();

    file->setPosFile(pos);

    file->write_byte(flag);
    file->write_next(strKey);
    file->write_next(strValue);
}

template<class K, class V>
void BTreeStore<K, V>::readEntry(Entry<K, V>* entry, const int& pos) {
    int key;
    int value;

    file->setPosFile(pos);

    char flag = file->read_byte();
    key = file->read_next<K>();
    value = file->read_next<V>();

    entry->setKeyValue(key, value);
}

template<class K, class V>
void BTreeStore<K, V> ::writeFlag(char flag, const int pos) {
    file->setPosFile(pos);

    file->write_byte(flag);
}

template<class K, class V>
void BTreeStore<K, V> ::insert(const Entry<K, V>* entry) {
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
        writeNode(root, root->m_pos);

        //write key value
        writeEntry(entry, pos);
    } else {
        if (root->nCurrentEntry == 2 * t - 1) {
            BTreeNodeStore<K, V>* newRoot = new BTreeNodeStore<K, V>(t, false);

            const int &pos1 = root->m_pos;
            newRoot->arrayPosChild[0] = pos1;

            file->setPosEndFile();

            const int &pos2 = file->getPosFile();
            newRoot->m_pos = pos2;
            //write node
            writeNode(newRoot, newRoot->m_pos);

            newRoot->splitChild(this, 0, root);
            //find child have new key
            int i = 0;
            Entry<K, V>* entryOfRoot = newRoot->getEntry(this, 0);
            if (entryOfRoot->getKey() < entry->getKey()) {
                i++;
            }
            delete entryOfRoot;

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
void BTreeStore<K, V> ::set(const K& key, const V& value) {
//    pthread_rwlock_wrlock(&(rwLock));
    //    int secs;
    //    timestamp_t timeFinish;
    //    timestamp_t timeStart = get_timestamp();

    if (root == NULL) {
        Entry<K, V>* entry = new Entry<K, V>(key, value);
        insert(entry);
    } else if (!root->set(this, key, value)) {
        Entry<K, V>* entry = new Entry<K, V>(key, value);
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

    Entry<K, V>* entry = root->search(this, key);

    if (entry == NULL) {
        res = false;
    } else {
        delete entry;
        res = true;
    }

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

