#pragma once

#include "btree.h"

template<typename K, typename V, typename Oit>
BTree<K, V, Oit>::BTree(int order) : root(new Node(order, true)) {
    assert(order >= 2);
}

template<typename K, typename V, typename Oit>
BTree<K, V, Oit>::~BTree() {
    delete root;
}

template<typename K, typename V, typename Oit>
bool BTree<K, V, Oit>::exist(const K &key) {
    return root->find_node(key) != nullptr;
}

template<typename K, typename V, typename Oit>
void BTree<K, V, Oit>::set(const K &key, const V& value) {
    Node* leaf_node = root->find_node(key);
    if (leaf_node != nullptr) {
        int pos = leaf_node->key_idx_in_node(key);
        leaf_node->set(pos, value);
    } else {
        insert(key, value);
    }
}

template<typename K, typename V, typename Oit>
bool BTree<K, V, Oit>::insert(const K& key, const V& value) {
    if (root->is_full()) {
        auto new_root = new Node(root->t, false);
        new_root->children[0] = root;
        new_root->split_child(0, root);

        // find the children have new key
        int pos = new_root->find_key_pos(key);
        new_root->children[pos]->insert_non_full(key, value);
        root = new_root;
    } else {
        root->insert_non_full(key, value);
    }
    return true;
}

template<typename K, typename V, typename Oit>
Oit BTree<K, V, Oit>::get(const K& key) {
    return std::istream_iterator<char>();
}

template<typename K, typename V, typename Oit>
bool BTree<K, V, Oit>::remove(const K& key) {
    return false;
}

template<typename K, typename V, typename Oit>
void BTree<K, V, Oit>::traverse() {
    if (root)
        root->traverse();
}