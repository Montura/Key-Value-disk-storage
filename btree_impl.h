#pragma once

#include "btree.h"

template<typename K, typename V, int t, typename Oit>
BTree<K, V, t, Oit>::BTree() : root(new Node(true)) {}

template<typename K, typename V, int t, typename Oit>
BTree<K, V, t, Oit>::~BTree() {
    delete root;
}

template<typename K, typename V, int t, typename Oit>
typename BTree<K, V, t, Oit>::Node* BTree<K, V, t, Oit>::find_leaf_node(const K& key) {
    Node* curr = root;
    while (!curr->is_leaf) {
        for (int i = 0; i <= curr->used_keys; ++i) {
            if (i == curr->used_keys || key <= curr->keys[i]) {
                curr = curr->children[i];
                break;
            }
        }
    }
    return curr;
}

template<typename K, typename V, int t, typename Oit>
bool BTree<K, V, t, Oit>::exist(const K &key) {
    Node* leaf_node = find_leaf_node(key);
    return leaf_node->has_key(key);
}

template<typename K, typename V, int t, typename Oit>
void BTree<K, V, t, Oit>::set(const K &key, const V& value) {
    Node* leaf_node = find_leaf_node(key);
    int pos = leaf_node->key_pos_in_leaf_node(key);
    if (pos != leaf_node->used_keys) {
        leaf_node->set(pos, value);
    } else {
        insert(key, value);
    }
}

template<typename K, typename V, int t, typename Oit>
bool BTree<K, V, t, Oit>::insert(const K& key, const V& value) {
    if (root->used_keys == max_key_num) {
        auto new_root = new Node(false);
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

template<typename K, typename V, int t, typename Oit>
Oit BTree<K, V, t, Oit>::get(const K& key) {
    return std::istream_iterator<char>();
}

template<typename K, typename V, int t, typename Oit>
bool BTree<K, V, t, Oit>::remove(const K& key) {
    return false;
}

template<typename K, typename V, int t, typename Oit>
void BTree<K, V, t, Oit>::traverse() {
    if (root)
        root->traverse();
}