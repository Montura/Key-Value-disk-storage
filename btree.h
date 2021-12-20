#pragma once

#include <vector>

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
    int t,
    typename Oit = std::istream_iterator<char>
>
class BTree final: IKeyValueStorage<K, V, Oit> {
    static constexpr int max_key_num = 2 * t - 1;
    static constexpr int max_child_num = 2 * t;

    /** Node */
    struct BTreeNode {
        uint8_t is_leaf = 0;
        std::vector<K> keys;
        std::vector<BTreeNode *> children;
        std::vector<V *> values; // only in leaves
        int used_keys = 0;

        BTreeNode() = delete;

    public:
        explicit BTreeNode(bool is_leaf = false);
        ~BTreeNode();

        int key_pos_in_leaf_node(const K& key) const;
        bool has_key(const K& key) const;
        int find_key_pos(const K& key) const;
        void set(int pos, const V& value);
        void insert_non_full(const K& key, const V& value);
        void split_child(const int& pos, BTreeNode* node);
        void traverse();
    };

    // Корень дерева содержит от (1 до 2t − 1) ключей (0 или от 2 до 2t детей).
    // Все остальные узлы содержат от (t − 1) до (2t − 1) ключей (от t до 2t детей).
    BTreeNode* root = nullptr;

public:
    using Node = BTreeNode;

    explicit BTree();
    ~BTree();

    Node* find_leaf_node(const K& key);
    bool exist(const K &key);
    void set(const K &key, const V& value);
    void traverse();

    bool insert(const K& key, const V& value) override;
    Oit get(const K& key) override;
    bool remove(const K& key) override;
};