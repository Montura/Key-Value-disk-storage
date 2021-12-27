#pragma once

#include "btree.h"
#include "entry.h"

template<class K, class V>
class BTreeStore;

template<class K, class V>
struct BTreeNodeStore {
    int nCurrentEntry;
    int t;
    char flag;
    int m_pos;
    std::vector<int> arrayPosKey;
    std::vector<int> arrayPosChild;

public:
    BTreeNodeStore(const int& t, bool isLeaf);

    BTreeNodeStore(const BTreeNodeStore& other) = delete;
    BTreeNodeStore operator=(const BTreeNodeStore& other) = delete;

    BTreeNodeStore(BTreeNodeStore && other) noexcept = default;
    BTreeNodeStore& operator=(BTreeNodeStore && other) noexcept = default;

    ~BTreeNodeStore();


    bool checkIsLeaf() const;

    int findKeyBinarySearch(BTreeStore<K, V>* bTree, const K& key);

    void splitChild(BTreeStore<K, V>* bTree, const int &index, BTreeNodeStore<K, V>& node);

    Entry<K, V> getEntry(BTreeStore<K, V>* bTree, const int &i);

    BTreeNodeStore<K, V> getBTreeNodeStore(BTreeStore<K, V>* bTree, const int &i);

    void insertNotFull(BTreeStore<K, V>* bTree, const Entry<K, V>& entry);

    void traverse(BTreeStore<K, V>* bTree);

    Entry<K, V> search(BTreeStore<K, V>* bTree, const K& key);

    bool set(BTreeStore<K, V> *bTree, const K &key, const V &value);

    bool remove(BTreeStore<K, V>* bTree, const K& key);
    bool removeFromLeaf(BTreeStore<K, V>* bTree, const int& index);
    bool removeFromNonLeaf(BTreeStore<K, V>* bTree, const int& index);

    int getPosEntryPred(BTreeStore<K, V>* bTree, const int& index);
    int getPosEntrySucc(BTreeStore<K, V>* bTree, const int& index);

    void mergeNode(BTreeStore<K, V>* bTree, const int& index);

    void fillNode(BTreeStore<K, V>* bTree, const int& index);

    void borrowFromNodePrev(BTreeStore<K, V>* bTree, const int &index);
    void borrowFromNodeNext(BTreeStore<K, V>* bTree, const int &index);

    inline int max_key_num() { return 2 * t - 1; }
    inline int max_child_num() { return 2 * t; }
};