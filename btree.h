#pragma once

#include <vector>
#include <cassert>
#include <mutex>

#include "entry.h"

template <typename K, typename V>
struct IOManager;

/** Tree */
template <typename K, typename V>
class BTree final {
    struct BTreeNode;
public:
    using Node = BTreeNode;
    using EntryT = Entry<K,V>;

    BTree(const std::string& path, int16_t order);
    ~BTree();

    bool exist(const K &key);
    void set(const K &key, const V& value);
    const V get(const K& key);
    bool remove(const K& key);

private:
    void insert(const K& key, const V& value);
    void traverse();

    BTreeNode* root = nullptr;
    const int16_t t;

    IOManager<K,V> io_manager;
    using IOManagerT = IOManager<K,V>;

    /** Node */
    struct BTreeNode final {
        int16_t used_keys;
        int16_t t;
        uint8_t flag;
        int64_t m_pos;
        std::vector<int64_t> key_pos;
        std::vector<int64_t> child_pos;

    public:
        BTreeNode(const int16_t& t, bool isLeaf);

        // todo: ugly code to ensure only move semantics
        BTreeNode(const BTreeNode& other) = delete;
        BTreeNode operator=(const BTreeNode& other) = delete;

        BTreeNode(BTreeNode && other) noexcept = default;
        BTreeNode& operator=(BTreeNode && other) noexcept = default;

        bool is_leaf() const;
        bool is_full() const;
        uint8_t is_deleted_or_is_leaf() const;

        inline int32_t max_key_num() const { return std::max(2 * t - 1, 0); }
        inline int32_t max_child_num() const { return 2 * t; }

        int32_t find_key_bin_search(IOManagerT& io_manager, const K& key);
        void split_child(IOManagerT& manager, const int32_t idx, Node& curr_node);

        K read_key(IOManagerT& io_manager, const int32_t idx);
        EntryT read_entry(IOManagerT& io_manager, const int32_t idx);
        EntryT find(IOManagerT& io_manager, const K& key);

        void insert_non_full(IOManagerT& io_manager, const K& key, const V& value);
        void traverse(IOManagerT& io_manager);
        bool set(IOManagerT& io_manager, const K &key, const V &value);
        bool remove(IOManagerT& io_manager, const K& key);
    private:
        Node read_node(IOManagerT& io_manager, const int32_t idx);

        bool remove_from_leaf(IOManagerT& io_manager, const int32_t idx);
        bool remove_from_non_leaf(IOManagerT& io_manager, const int32_t idx);

        int64_t get_prev_entry_pos(IOManagerT& io_manager, const int32_t idx);
        int64_t get_next_entry_pos(IOManagerT& io_manager, const int32_t idx);

        void merge_node(IOManagerT& io_manager, const int32_t idx);
        int32_t fill_node(IOManagerT& io_manager, const int32_t idx);

        void borrow_from_prev_node(IOManagerT& io_manager, const int32_t idx);
        void borrow_from_next_node(IOManagerT& io_manager, const int32_t idx);
    };
};