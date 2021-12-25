#pragma once

#include <vector>
#include <iterator>
#include <cassert>
#include <optional>
#include <mutex>

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
class BTree final {
    struct BTreeNode;

    BTreeNode* root = nullptr;
    const int t;

    MappedFile file;
public:
    using Node = BTreeNode;

    explicit BTree(const std::string& path, int order);
    ~BTree();

    bool exist(const K &key);
    void set(const K &key, const V& value);
    void traverse();

    void insert(const Entry<K, V>& entry);
    const V get(const K& key);
    bool remove(const K& key);

// todo: extract to reader|writer

    void write_entry(const Entry<K, V>& entry, const int pos);
    void read_entry(Entry<K, V>& entry, const int pos);

    void write_flag(char flag, const int pos);

    void write_header(const int t, const int posRoot);
    void read_header(int& t, int& posRoot);

    void read_node(Node* node, const int pos);
    void write_node(const Node& node, const int pos);

    int getPosFileWrite();
    void setPosEndFileWrite();
    void writeUpdatePosRoot(const int posRoot);

    int calc_node_writable_node_size();
private:
    /** Node */
    struct BTreeNode {
        int used_keys;
        int t;
        char flag;
        int m_pos;
        std::vector<int> arrayPosKey;
        std::vector<int> arrayPosChild;

    public:
        BTreeNode(const int& t, bool isLeaf);

        BTreeNode(const BTreeNode& other) = delete;
        BTreeNode operator=(const BTreeNode& other) = delete;

        BTreeNode(BTreeNode && other) noexcept = default;
        BTreeNode& operator=(BTreeNode && other) noexcept = default;

        ~BTreeNode();

        bool is_leaf() const;

        bool is_full() const;

        int find_key_bin_search(BTree* bTree, const K& key);

        void split_child(BTree* bTree, const int index, Node& node);

        Entry<K, V> read_entry(BTree* bTree, const int i);

        void insert_non_full(BTree* bTree, const Entry<K, V>& entry);

        void traverse(BTree* bTree);

        bool set(BTree *bTree, const K &key, const V &value);
        bool remove(BTree* bTree, const K& key);
        Entry<K, V> find(BTree* bTree, const K& key);

        inline int max_key_num() const { return 2 * t - 1; }
        inline int max_child_num() const { return 2 * t; }
    private:
        Node get_node(BTree* bTree, const int i);

        bool remove_from_leaf(BTree* bTree, const int index);
        bool remove_from_non_leaf(BTree* bTree, const int index);

        int get_prev_entry_pos(BTree* bTree, const int index);
        int get_next_entry_pos(BTree* bTree, const int index);

        void merge_node(BTree* bTree, const int index);
        void fill_node(BTree* bTree, const int index);

        void borrow_from_node_prev(BTree* bTree, const int index);
        void borrow_from_node_next(BTree* bTree, const int index);
    };
};