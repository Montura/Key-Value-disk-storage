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
    std::vector<int> arrayPosKey;
    std::vector<int> arrayPosChild;
    int m_pos;

public:
    BTreeNodeStore(const int& t, bool isLeaf);

    ~BTreeNodeStore();

    void addPosEntry(const int &i, const int &pos);
    void addPosChild(const int &i, const int &pos);

    void increaseNCurrentEntry();

    int getPos() const;
    void setPost(const int& pos);

    char getFlag() const;
    void setFlag(char flag);

    int getNCurrentEntry() const;
    void setNCurrentEntry(const int& nCurrentEntry);

    int getPosChild(const int &i);
    int getPosEntry(const int &i) const;


    bool checkIsLeaf() const;

    int findKeyBinarySearch(BTreeStore<K, V>* bTree, const K& key);

    void splitChild(BTreeStore<K, V>* bTree, const int &index, BTreeNodeStore<K, V>* &node);

    std::optional<Entry<K, V>> getEntry(BTreeStore<K, V>* bTree, const int &i);

    BTreeNodeStore<K, V>* getBTreeNodeStore(BTreeStore<K, V>* bTree, const int &i);

    void insertNotFull(BTreeStore<K, V>* bTree, const Entry<K, V>& entry);

    void traverse(BTreeStore<K, V>* bTree);

    std::optional<Entry<K, V>> search(BTreeStore<K, V>* bTree, const K& key);

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