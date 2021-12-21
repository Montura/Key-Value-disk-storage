#pragma once

#include <vector>
#include <iterator>
#include <cassert>

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
class BTree final: IKeyValueStorage<K, V, Oit> {
    struct BTreeNode;
    BTreeNode* root = nullptr;

public:
    using Node = BTreeNode;

    explicit BTree(int order);
    ~BTree();

    bool exist(const K &key);
    void set(const K &key, const V& value);
    void traverse();

    bool insert(const K& key, const V& value) override;
    Oit get(const K& key) override;
    bool remove(const K& key) override;

private:
    /** Node */
    struct BTreeNode {
        const uint8_t t;
        int used_keys = 0;
        uint8_t is_leaf = 0;
        std::vector<K> keys;
        std::vector<V *> values;
        std::vector<BTreeNode *> children;

        BTreeNode() = delete;
        explicit BTreeNode(int t, bool is_leaf);
        ~BTreeNode();

        BTreeNode* find_node(const K &key);
        int key_idx_in_node(const K& key) const;
        int find_key_pos(const K& key) const;
        void set(int pos, const V& value);
        void insert_non_full(const K& key, const V& value);
        void split_child(const int& pos, BTreeNode* node);
        void traverse();

        inline bool is_full() { return used_keys == max_key_num(); }
        inline int max_key_num() { return 2 * t - 1; }
        inline int max_child_num() { return 2 * t; }
    };
};