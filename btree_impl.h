#pragma once

#include "btree.h"
#include "btree_node.h"
#include "byte_data.h"

template<class K, class V>
BTreeStore<K, V> ::BTreeStore(const std::string& path, int order) : PATH(path), t(order) {
    manageFileWrite = new ManageFileWrite();
    manageFileRead = new ManageFileRead();

    int fdw = manageFileWrite->openFile(PATH);
    if (fdw < 0) {
        exit(EXIT_FAILURE);
    }

    int fdr = manageFileRead->openFile(PATH);
    if (fdr < 0) {
        exit(EXIT_FAILURE);
    }

    writeDisk = new BytesDataOutputStore(MAXLEN, fdw);
    readDisk = new BytesDataInputStore(MAXLEN, fdr);

//    pthread_rwlock_init(&(rwLock), NULL);

    if (manageFileRead->isEmptyFile()) {
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
    delete manageFileRead;
    delete manageFileWrite;
    delete writeDisk;
    delete readDisk;

//    pthread_rwlock_destroy(&(rwLock));
}

template<class K, class V>
void BTreeStore<K, V>::readHeader(int& t, int& posRoot) {
    manageFileRead->setPosFile(0);

    readDisk->resetBuffer();
    readDisk->readInt(t);
    readDisk->readInt(posRoot);
}

template<class K, class V>
void BTreeStore<K, V> ::writeHeader(const int t, const int posRoot) {
    manageFileWrite->setPosFile(0);

    writeDisk->writeInt(t);
    writeDisk->writeInt(posRoot);
    writeDisk->flush();
}

template<class K, class V>
void BTreeStore<K, V> ::writeUpdatePosRoot(const int posRoot) {
    manageFileWrite->setPosFile(4);

    writeDisk->writeInt(posRoot);
    writeDisk->flush();
}

template<class K, class V>
void BTreeStore<K, V> ::writeNode(BTreeNodeStore<K, V>* node, const int pos) {
    manageFileWrite->setPosFile(pos);
    int* arrPosKey = node->getArrayPosKey();
    int* arrPosChild = node->getArrayPosChild();
    char flag = node->getFlag();
    int nCurrentEntry = node->getNCurrentEntry();
    int size = 2 * t;

    writeDisk->writeByte(flag);
    writeDisk->writeInt(nCurrentEntry);
    writeDisk->writeArrayInt(arrPosKey, size - 1);
    writeDisk->writeArrayInt(arrPosChild, size);
    writeDisk->flush();
}

template<class K, class V>
void BTreeStore<K, V>::readNode(BTreeNodeStore<K, V>* node, const int pos) {
    int nCurrentKey;
    char flag;
    int size = 2 * t;
    int *arrPosKey = new int[size - 1];
    int *arrPosChild = new int[size];

    manageFileRead->setPosFile(pos);
    readDisk->resetBuffer();

    readDisk->readByte((uint8_t&) flag);
    readDisk->readInt(nCurrentKey);
    readDisk->readArrayInt(arrPosKey, size - 1);
    readDisk->readArrayInt(arrPosChild, size);

    node->setFlag(flag);
    node->setPost(pos);
    node->setMinimumDegre(t);
    node->setNCurrentEntry(nCurrentKey);
    node->setArrayPosKey(arrPosKey);
    node->setArrayPosChild(arrPosChild);
}

template<class K, class V>
void BTreeStore<K, V>::writeEntry(const Entry<K, V>& entry, const int pos) {
    char flag = 1;
    int strKey = entry.key;
    int strValue = entry.value;

//    string key = mySerialization->serializationKey(strKey);
//    string value = mySerialization->serializationValue(strValue);

    manageFileWrite->setPosFile(pos);
    //write flag: danh dau da remove or active or edit
    writeDisk->writeByte(flag);
    //write key
    writeDisk->writeBytes((uint8_t*) &strKey, 0, 4);
    //write value
    writeDisk->writeBytes((uint8_t*) &strValue, 0, 4);

    writeDisk->flush();
    //
    //    delete[] key;
    //    delete[] value;
}

template<class K, class V>
void BTreeStore<K, V>::readEntry(Entry<K, V>& entry, const int pos) {
    int key;
    int value;
    char flag;
//    int lenKey, lenValue;

    manageFileRead->setPosFile(pos);
    readDisk->resetBuffer();

    readDisk->readByte((uint8_t&) flag);

//    readDisk->readBytes(reinterpret_cast<uint8_t*> (&lenKey), 0, sizeof (lenKey));
//    key.resize(lenKey);
    readDisk->readBytes((uint8_t*) &key, 0, 4);

//    readDisk->readBytes(reinterpret_cast<uint8_t*> (&lenValue), 0, sizeof (lenValue));
//    value.resize(lenValue);
    readDisk->readBytes((uint8_t*) &value, 0, 4);

    entry.setKeyValue(key, value);
}

template<class K, class V>
void BTreeStore<K, V> ::writeFlag(char flag, const int pos) {
    manageFileWrite->setPosFile(pos);

    writeDisk->writeByte(flag);
    writeDisk->flush();
}

template<class K, class V>
void BTreeStore<K, V> ::insert(const Entry<K, V>& entry) {
    if (root == NULL) {
        root = new BTreeNodeStore<K, V>(t, true);
        writeHeader(t, 8);
        root->setPost(8);

        manageFileWrite->setPosEndFile();
        int pos = manageFileWrite->getPosFile();
        pos = pos + sizeof (int) * (2 * t - 1) + sizeof (int) * (2 * t) + 5; // 1 byte flag + 4 byte nCurrentKey

        root->addPosEntry(0, pos);
        root->setFlag(1);
        root->increaseNCurrentEntry();

        //write node root
        writeNode(root, root->getPos());

        //write key value
        writeEntry(entry, pos);
    } else {
        if (root->getNCurrentEntry() == 2 * t - 1) {
            BTreeNodeStore<K, V>* newRoot = new BTreeNodeStore<K, V>(t, false);

            newRoot->addPosChild(0, root->getPos());

            manageFileWrite->setPosEndFile();

            newRoot->setPost(manageFileWrite->getPosFile());
            //write node
            writeNode(newRoot, newRoot->getPos());

            newRoot->splitChild(this, 0, root);
            //find child have new key
            int i = 0;
            Entry<K, V> entryOfRoot = newRoot->getEntry(this, 0).value();
            if (entryOfRoot.key < entry.key) {
                i++;
            }

            BTreeNodeStore<K, V>* node = new BTreeNodeStore<K, V>(t, false);
            int pos = newRoot->getPosChild(i);

            //read node
            readNode(node, pos);

            node->insertNotFull(this, entry);

            readNode(root, newRoot->getPos());

            //cap nhat lai header
            writeUpdatePosRoot(newRoot->getPos());

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

    if (root->getNCurrentEntry() == 0) {
        if (root->checkIsLeaf()) {
            char flag = root->getFlag();
            flag = flag | (1 << 1);
            writeFlag(flag, root->getPos());
            delete root;
            root = NULL;
            writeUpdatePosRoot(-1);
        } else {
            int pos = root->getPosChild(0);
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
    return manageFileWrite->getPosFile();
}

template<class K, class V>
void BTreeStore<K, V>::setPosEndFileWrite() {
    manageFileWrite->setPosEndFile();
}

