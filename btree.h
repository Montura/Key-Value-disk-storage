#pragma once

#include <vector>
#include <cassert>
#include <mutex>

#include "entry.h"

template <typename K, typename V>
class IOManager;

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
    V get(const K& key);
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
        uint8_t is_leaf;
        int64_t m_pos;
        std::vector<int64_t> key_pos;
        std::vector<int64_t> child_pos;

    public:
        BTreeNode(const int16_t& t, bool isLeaf);

        EntryT find(IOManagerT& io_manager, const K& key);
        bool set(IOManagerT& io_manager, const K &key, const V &value);
        K get_key(IOManagerT& io_manager, const int32_t idx);
        bool remove(IOManagerT& io_manager, const K& key);

        bool is_full() const;
        void split_child(IOManagerT& manager, const int32_t idx, Node& curr_node);
        void insert_non_full(IOManagerT& io_manager, const K& key, const V& value);
//        void traverse(IOManagerT& io_manager);
    private:
        int32_t max_key_num() const;
        int32_t max_child_num() const;

        int32_t find_key_bin_search(IOManagerT& io_manager, const K& key);

        Entry<K, V> get_entry(IOManagerT& io_manager, const int32_t idx);
        Node get_child(IOManagerT& io_manager, const int32_t idx);

        bool remove_from_leaf(IOManagerT& io_manager, const int32_t idx);
        bool remove_from_non_leaf(IOManagerT& io_manager, const int32_t idx);

        int64_t get_prev_entry_pos(IOManagerT& io_manager, const int32_t idx);
        int64_t get_next_entry_pos(IOManagerT& io_manager, const int32_t idx);

        void merge_node(IOManagerT& io_manager, const int32_t idx);
        void fill_node(IOManagerT& io_manager, const int32_t idx);

        void borrow_from_prev_node(IOManagerT& io_manager, const int32_t idx);
        void borrow_from_next_node(IOManagerT& io_manager, const int32_t idx);
    };
};

#include "io_manager.h"
#include "btree_impl.h"
#include "btree_node_impl.h"