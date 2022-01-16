#pragma once

#include <vector>

#include "utils/forward_decl.h"

namespace btree {
    template <typename K, typename V>
    struct BTreeNode final {
        int16_t used_keys;
        int16_t t;
        uint8_t is_leaf;
        int64_t m_pos;
        int64_t* key_pos;
        int64_t* child_pos;

        using Node = BTreeNode;
        using EntryT = typename BTree<K,V>::EntryT;
        using IOManagerT = IOManager<K, V>;

        explicit BTreeNode();
        BTreeNode(const int16_t& t, bool isLeaf);

        BTreeNode(const BTreeNode& other) {
            if (this != &other) {
                used_keys = other.used_keys;
                t = other.t;
                is_leaf = other.is_leaf;
                m_pos = other.m_pos;
                key_pos = new int64_t[max_key_num(t)];
                child_pos = new int64_t[max_child_num(t)];
                std::copy(other.key_pos, other.key_pos + max_key_num(t), key_pos);
                std::copy(other.child_pos, other.child_pos + max_child_num(t), child_pos);
            }
        }

        BTreeNode& operator=(const BTreeNode& other) {
            if (this != &other) {
                used_keys = other.used_keys;
                t = other.t;
                is_leaf = other.is_leaf;
                m_pos = other.m_pos;
                key_pos = new int64_t[max_key_num(t)];
                child_pos = new int64_t[max_child_num(t)];
                std::copy(other.key_pos, other.key_pos +  max_key_num(t), key_pos);
                std::copy(other.child_pos, other.child_pos +  max_child_num(t), child_pos);
            }
            return *this;
        }

        BTreeNode(BTreeNode && other) noexcept {
            if (this != &other) {
                other.swap(*this);
            }
        }

        BTreeNode& operator=(BTreeNode && other) noexcept {
            if (this != &other) {
                other.swap(*this);
            }
            return *this;
        }

        void swap(BTreeNode & other) noexcept {
            std::swap(used_keys, other.used_keys);
            std::swap(t, other.t);
            std::swap(is_leaf, other.is_leaf);
            std::swap(m_pos, other.m_pos);
            std::swap(key_pos, other.key_pos);
            std::swap(child_pos, other.child_pos);
        }

        ~BTreeNode() {
            delete[] key_pos;
            delete[] child_pos;
        }

        bool set(IOManagerT& io_manager, const EntryT& e);
        bool remove(IOManagerT& io_manager, const K key);

        EntryT find(IOManagerT& io_manager, const K key) const;
        K get_key(IOManagerT& io_manager, const int32_t idx) const;

        static constexpr int32_t get_node_size_in_bytes(const int16_t t);
        bool is_full() const;
        bool is_valid() const;

        void split_child(IOManagerT& manager, const int32_t idx, BTreeNode& curr_node);
        void insert_non_full(IOManagerT& io_manager, const EntryT& e);
        static constexpr int32_t max_key_num(const int16_t t);
        static constexpr int32_t max_child_num(const int16_t t);
    private:

        int32_t find_key_bin_search(IOManagerT& io_manager, const K key) const;
        std::tuple<BTreeNode, EntryT, int32_t> find_leaf_node_with_key(IOManagerT& io_manager, const K key) const;

        EntryT get_entry(IOManagerT& io_manager, const int32_t idx) const;
        BTreeNode get_child(IOManagerT& io_manager, const int32_t idx) const;

        bool remove_from_leaf(IOManagerT& io_manager, const int32_t idx);
        bool remove_from_non_leaf(IOManagerT& io_manager, const int32_t idx);

        int64_t get_prev_entry_pos(IOManagerT& io_manager, const int32_t idx) const;
        int64_t get_next_entry_pos(IOManagerT& io_manager, const int32_t idx) const;

        void merge_node(IOManagerT& io_manager, const int32_t idx);
        void fill_node(IOManagerT& io_manager, const int32_t idx);

        void borrow_from_prev_node(IOManagerT& io_manager, const int32_t idx);
        void borrow_from_next_node(IOManagerT& io_manager, const int32_t idx);
    };
}

#include "btree_impl/btree_node_impl.h"
