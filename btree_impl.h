#pragma once

#include "btree.h"
#include "byte_data.h"

template<typename K, typename V, typename Oit>
BTree<K, V, Oit>::BTree(int order) {
    assert(order >= 2);
    t = order;

    manageFileWrite = new ManageFileWrite();
    manageFileRead = new ManageFileRead();

    int fdr, fdw, pos;

    fdw = manageFileWrite->openFile(PATH);
    if (fdw < 0) {
        exit(EXIT_FAILURE);
    }

    fdr = manageFileRead->openFile(PATH);
    if (fdr < 0) {
        exit(EXIT_FAILURE);
    }

    writeDisk = new BytesDataOutputStore(MAXLEN, fdw);
    readDisk = new BytesDataInputStore(MAXLEN, fdr);

//    mySerialization = new StringSerialization();

//    pthread_rwlock_init(&(rwLock), NULL);

    if (manageFileRead->isEmptyFile()) {
        root = NULL;
        return;
    }
    readHeader(t, pos);

    if (pos == -1) {
        root = NULL;
        return;
    }

    root = new BTreeNode(t, true);
    readNode(root, pos);
}

template<typename K, typename V, typename Oit>
BTree<K, V, Oit>::~BTree() {
    delete root;
    delete manageFileRead;
    delete manageFileWrite;
    delete writeDisk;
    delete readDisk;
}

template<typename K, typename V, typename Oit>
bool BTree<K, V, Oit>::exist(const K &key) {
    if (root == nullptr) {
        return false;
    }

    auto optional = root->search(this, key);
    return optional.has_value();
}

template<typename K, typename V, typename Oit>
void BTree<K, V, Oit>::set(const K &key, const V& value) {
    if (root == nullptr) {
        insert(key, value);
    } else if (!root->set(this, key, value)) {
        insert(key, value);
    }
}

template<typename K, typename V, typename Oit>
bool BTree<K, V, Oit>::insert(const K& key, const V & value) {
    int pos = -1;
    if (root == nullptr) {
        root = new BTreeNode(t, true);
        writeHeader(t, 8);
        root->pos = 8;

        manageFileWrite->setPosEndFile();
        pos = manageFileWrite->getPosFile();
        pos = pos + sizeof (int) * (2 * t - 1) + sizeof (int) * (2 * t) + 5; // 1 byte flag + 4 byte nCurrentKey

        root->keys[0] = pos;
        root->flag = 1;
        ++root->used_keys;

        // write node root
        writeNode(root, root->pos);

        //write key value
        write_key_value(pos, key, value);
    } else {
        if (root->is_full()) {
            auto new_root = new Node(root->t, false);
            
            new_root->children_idx[0] = root->pos;
            manageFileWrite->setPosEndFile();

            new_root->pos = manageFileWrite->getPosFile();
            //write node
            writeNode(new_root, new_root->pos);

            new_root->split_child(this, 0, root);

            //find child have new key
            int i = 0;
            auto optional = new_root->get_key_value(this, 0);
            assert(optional.has_value());
            auto [root_key, root_value] = optional.value();

            if (root_key < key) {
                ++i;
            }

            Node* node = new Node(t, false);
            pos = new_root->children_idx[i];

            //read node
            readNode(node, pos);

            node->insert_non_full(this, key, value);

            readNode(root, new_root->pos);
            //cap nhat lai header
            writeUpdatePosRoot(new_root->pos);
            delete new_root;
            delete node;
        } else {
            root->insert_non_full(this, key, value);
        }
    }
    return true;
}

template<typename K, typename V, typename Oit>
Oit BTree<K, V, Oit>::get(const K& key) {
    return std::istream_iterator<char>();
}

template<typename K, typename V, typename Oit>
bool BTree<K, V, Oit>::remove(const K& key) {
    if (!root) {
//        cout << "The tree is empty\n";
        return false;
    }
    bool success = root && root->remove(this, key);
    if (success && root->used_keys == 0) {
        if (root->is_leaf) {
            char flag = root->flag;
            flag = flag | (1 << 1);
            writeFlag(flag, root->pos);
            delete root;
            root = nullptr;
            writeUpdatePosRoot(-1);
        } else {
            int pos =root->children_idx[0];
            writeUpdatePosRoot(pos);
            readNode(root, pos);
        }
    }
    return success;
}

template<typename K, typename V, typename Oit>
void BTree<K, V, Oit>::traverse() {
    if (root)
        root->traverse(this);
}

template<typename K, typename V, typename Oit>
bool BTree<K, V, Oit>::readHeader(int& t, int& posRoot) {
    manageFileRead->setPosFile(0);

    readDisk->resetBuffer();
    readDisk->readInt(t);
    readDisk->readInt(posRoot);
}

template<typename K, typename V, typename Oit>
bool BTree<K, V, Oit>::readNode(Node* node, const int pos) {
    int used_keys = 0;
    char flag = 0;
    int size = 2 * t;
    int *arrPosKey = new int[size - 1];
    int *arrPosChild = new int[size];

    manageFileRead->setPosFile(pos);
    readDisk->resetBuffer();

    readDisk->readByte((uint8_t&) flag);
    readDisk->readInt(used_keys);
    readDisk->readArrayInt(arrPosKey, size - 1);
    readDisk->readArrayInt(arrPosChild, size);

    node->flag = flag & 1;
    node->pos = pos;
    node->used_keys = used_keys;
    node->keys.assign(arrPosKey, arrPosKey + max_key_num());
    node->children_idx.assign(arrPosChild, arrPosChild + max_child_num());
    delete[] arrPosKey;
    delete[] arrPosChild;
}

template <typename K, typename V, typename Oit>
void BTree<K, V, Oit>::writeMinimumDegree(const int &t) const {
    writeDisk->writeInt(t);
    writeDisk->flush();
}


template <typename K, typename V, typename Oit>
void BTree<K, V, Oit> ::writeHeader(const int& t, const int& posRoot) {
    manageFileWrite->setPosFile(0);

    writeDisk->writeInt(t);
    writeDisk->writeInt(posRoot);
    writeDisk->flush();
}

template <typename K, typename V, typename Oit>
void BTree<K, V, Oit> ::writeUpdatePosRoot(const int& posRoot) {
    manageFileWrite->setPosFile(4);

    writeDisk->writeInt(posRoot);
    writeDisk->flush();
}

template <typename K, typename V, typename Oit>
void BTree<K, V, Oit>::writeNode(Node* node, const int pos) {
    manageFileWrite->setPosFile(pos);
    K* arrPosKey = node->keys.data();
    K* arrPosChild = node->children_idx.data();
    char flag = node->flag;
    int used_keys = node->used_keys;
    int size = 2 * t;

    writeDisk->writeByte(flag);
    writeDisk->writeInt(used_keys);
    writeDisk->writeArrayInt(arrPosKey, size - 1);
    writeDisk->writeArrayInt(arrPosChild, size);
    writeDisk->flush();
}

template <typename K, typename V, typename Oit>
void BTree<K, V, Oit>::writeFlag(char flag, const int& pos) {
    manageFileWrite->setPosFile(pos);

    writeDisk->writeByte(flag);
    writeDisk->flush();
}

template <typename K, typename V, typename Oit>
std::pair<K, V> BTree<K, V, Oit>::read_key_value(const int& pos) {
    K key;
    V value;
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

    return std::make_pair(key, value);
}

template <typename K, typename V, typename Oit>
void BTree<K, V, Oit>::write_key_value(const int& pos, const K& key, const V& value) {
    char flag = 1;

    manageFileWrite->setPosFile(pos);
    //write flag: danh dau da remove or active or edit
    writeDisk->writeByte(flag);
    //write key
    writeDisk->writeBytes((uint8_t*) &key, 0, 4);
    //write value
    writeDisk->writeBytes((uint8_t*) &value, 0, 4);

    writeDisk->flush();
    //
    //    delete[] key;
    //    delete[] value;
}


template <typename K, typename V, typename Oit>
int BTree<K, V, Oit>::getPosFileRead() const {

    return manageFileRead->getPosFile();
}

template <typename K, typename V, typename Oit>
void BTree<K, V, Oit>::setPosFileRead(const int& i) {

    manageFileRead->setPosFile(i);
}

template <typename K, typename V, typename Oit>
void BTree<K, V, Oit>::setPosEndFileRead() {

    manageFileRead->setPosEndFile();
}

template <typename K, typename V, typename Oit>
int BTree<K, V, Oit>::getPosFileWrite() const {

    return manageFileWrite->getPosFile();
}

template <typename K, typename V, typename Oit>
void BTree<K, V, Oit>::setPosEndFileWrite() {

    manageFileWrite->setPosEndFile();
}

template <typename K, typename V, typename Oit>
void BTree<K, V, Oit>::setPosFileWrite(const int& i) {
    manageFileWrite->setPosFile(i);
}