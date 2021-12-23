#pragma once

#include <vector>
#include <iterator>
#include <cassert>
#include <optional>

#include "serialization.h"
#include "manage_file.h"
#include "data_writer.h"

/** Common Interface */
template <typename Key, typename Value, typename Oit>
struct IKeyValueStorage {
    virtual bool insert(const Key& key, const Value& value) = 0;
    virtual Oit get(const Key& key) = 0;
    virtual bool remove(const Key& key) = 0;

    virtual ~IKeyValueStorage() { };
};

/** Tree */
template <  typename K,
    typename V,
    typename Oit = std::istream_iterator<char>
>
class BTree final {
    struct BTreeNode;
    const int MAXLEN = 1024;
    const char* PATH = "../database.txt";

    BTreeNode* root = nullptr;
    int t;

    ManageFile* manageFileRead;
    ManageFile* manageFileWrite;
    DataOutput* writeDisk;
    DataInput* readDisk;

public:
    using Node = BTreeNode;

    explicit BTree(int order);
    ~BTree();

    bool exist(const K &key);
    void set(const K &key, const V& value);
    void traverse();

    bool insert(const K& key, const V& value);
    Oit get(const K& key);
    bool remove(const K& key);

private:

    void write_key_value(const int& pos, const K& key,const V& value);
    std::pair<K, V> read_key_value(const int& pos);

    void writeFlag(char flag, const int &pos);

    bool readHeader(int& t, int& posRoot);
    bool readNode(Node* node, const int pos);

    int getPosFileRead() const;
    void setPosFileRead(const int& i);
    void setPosEndFileRead();

    int getPosFileWrite() const;
    void setPosFileWrite(const int& i);
    void setPosEndFileWrite();

    void writeNode(Node* node, const int pos);
    void writeMinimumDegree(const int &t) const;
    void writeHeader(const int &t, const int &posRoot);
    void writeHeaderNode(const char &flag, const int &nCurrentKey);
    void writeUpdatePosRoot(const int &posRoot);

    inline int max_key_num() { return 2 * t - 1; }
    inline int max_child_num() { return 2 * t; }
private:
    /** Node */
    struct BTreeNode {
        const uint8_t t;
        int pos;

        int used_keys = 0;
        char flag = 0;

//        bool is_leaf = false;
//        bool is_deleted = false;
        std::vector<K> keys;
//        std::vector<V*> values;
//        std::vector<BTreeNode*> children;
        std::vector<K> children_idx;

        BTreeNode() = delete;
        explicit BTreeNode(int t, bool is_leaf);
        ~BTreeNode();

//        BTreeNode* find_node(const K &key);
//        int key_idx_in_node(const K& key) const;
//        int find_key_pos(const K& key) const;
        std::optional<std::pair<K, V>> search(BTree* tree, const K& key);
        bool set(BTree* tree, const K& key, const V& value);
        void insert_non_full(BTree* tree, const K& key, const V& value);
        void split_child(BTree* tree, const int& idx, BTreeNode* node);
        void traverse(BTree* tree);
        bool remove(BTree* tree, const K& idx);

        inline bool is_full() { return used_keys == max_key_num(); }

        std::optional<std::pair<K,V>> get_key_value(BTree* tree, const int pos);
        int find_key_binary_search(BTree *tree, const K& key);

        BTreeNode* getBTreeNodeStore(BTree* bTree, const int& i);

        bool checkIsLeaf() const {
            return (flag & 1) == 1;
        }
    private:
        bool remove_from_leaf(BTree* tree, const int idx);
        bool remove_from_non_leaf(BTree* tree, const int idx);
        int get_previous_key_value(BTree* tree, const int idx) const;
        int get_next_key_value(BTree* tree, const int idx) const;
        void fill_or_merge_node(BTree* tree, const int idx);
        void merge_node(BTree* tree, const int idx);
        void borrow_from_node_prev(BTree* tree, const int idx);
        void borrow_from_node_next(BTree* tree, const int idx);

        inline int max_key_num() { return 2 * t - 1; }
        inline int max_child_num() { return 2 * t; }
    };
};