#pragma once

#include <vector>
#include <iterator>
#include <cassert>
#include <optional>
#include <mutex>

#include "manage_file.h"
#include "data_writer.h"
#include "btree_node.h"
#include "file_mapping.h"

/** Common Interface */
template <typename Key, typename Value, typename Oit>
struct IKeyValueStorage {
    virtual bool insert(const Key& key, const Value& value) = 0;
    virtual Oit get(const Key& key) = 0;
    virtual bool remove(const Key& key) = 0;

    virtual ~IKeyValueStorage() { };
};

/** Tree */
template <typename K, typename V>
class BTreeStore final {
//    struct BTreeNode;
    const int MAXLEN = 1024;
    const std::string PATH;

    BTreeNodeStore<K,V>* root = nullptr;
    const int t;

//    ManageFile* manageFileRead;
//    ManageFile* manageFileWrite;
//    DataOutput* writeDisk;
//    DataInput* readDisk;
    MappedFile* file;

public:
//    using Node = BTreeNode;

    explicit BTreeStore(const std::string& path, int order);
    ~BTreeStore();

    bool exist(const K &key);
    void set(const K &key, const V& value);
    void traverse();

    void insert(const Entry<K, V>& entry);
//    Oit get(const K& key);
    bool remove(const K& key);

//private:

    void writeEntry(const Entry<K, V>& entry, const int pos);
    void readEntry(Entry<K, V>& entry, const int pos);

    void writeFlag(char flag, const int pos);

    void readHeader(int& t, int& posRoot);
    void readNode(BTreeNodeStore<K,V>* node, const int pos);

    int getPosFileWrite() const;
    void setPosEndFileWrite();

    void writeNode(BTreeNodeStore<K,V>* node, const int pos);
    void writeHeader(const int t, const int posRoot);
    void writeUpdatePosRoot(const int posRoot);

    inline int max_key_num() { return 2 * t - 1; }
    inline int max_child_num() { return 2 * t; }
private:
    /** Node */
//    struct BTreeNode {
//        const uint8_t t;
//        int m_pos;
//
//        int nCurrentEntry = 0;
//        char flag = 0;
//
////        bool is_leaf = false;
////        bool is_deleted = false;
//        K* arrayPosKey;
////        std::vector<V*> values;
////        std::vector<BTreeNode*> children;
//        K* arrayPosChild;
//
//        BTreeNode() = delete;
//        explicit BTreeNode(int t, bool is_leaf);
//        ~BTreeNode();
//
////        BTreeNode* find_node(const K &key);
////        int key_idx_in_node(const K& key) const;
////        int find_key_pos(const K& key) const;
//        Entry<K, V>* search(BTreeStore* tree, const K& key);
//        bool set(BTreeStore* tree, const K& key, const V& value);
//        void insertNotFull(BTreeStore* tree, const Entry<K, V>* entry);
//        void splitChild(BTreeStore* tree, const int idx, BTreeNode* node);
//        void traverse(BTreeStore* tree);
//        bool remove(BTreeStore* tree, const K& idx);
//
//        BTreeNode* getNode(BTreeStore* bTree, const int i);
//
////        inline bool is_full() { return used_keys == max_key_num(); }
//
//        Entry<K, V>* getEntry(BTreeStore* tree, const int m_pos);
//        int findKeyBinarySearch(BTreeStore *tree, const K& key);
//
//        BTreeNode* getBTreeNodeStore(BTreeStore* bTree, const int i);
//
//        bool checkIsLeaf() const;
//
//        void addPosEntry(const int &i, const int &m_pos);
//        void addPosChild(const int &i, const int &m_pos);
//
//        void increaseNCurrentEntry();
//
//        K* getArrayPosKey();
//        void setArrayPosKey(int* arrPosKey);
//        K* getArrayPosChild();
//        void setArrayPosChild(int* arrPosChild);
//
//        int getPos() const;
//        void setPost(const int& m_pos);
//
//        char getFlag() const;
//        void setFlag(char flag);
//
//        int getNCurrentEntry() const;
//        void setNCurrentEntry(const int& nCurrentEntry);
//
//        int getPosChild(const int &i);
//        int getPosEntry(const int &i) const;
//
//        void setMinimumDegre(const int& t);
//        int getMinimumDegre() const;
//
//    private:
//        bool removeFromLeaf(BTreeStore* tree, const int idx);
//        bool removeFromNonLeaf(BTreeStore* tree, const int idx);
//        int getPosEntryPred(BTreeStore* tree, const int idx) const;
//        int getPosEntrySucc(BTreeStore* tree, const int idx) const;
//        void fillNode(BTreeStore* tree, const int idx);
//        void mergeNode(BTreeStore* tree, const int idx);
//        void borrowFromNodePrev(BTreeStore* tree, const int idx);
//        void borrowFromNodeNext(BTreeStore* tree, const int idx);
//
//        inline int max_key_num() { return 2 * t - 1; }
//        inline int max_child_num() { return 2 * t; }
//    };
};