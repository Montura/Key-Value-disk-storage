#pragma once

#include "btree.h"

template<typename K, typename V, typename Oit>
BTree<K, V, Oit>::BTreeNode::BTreeNode(int order, bool is_leaf):
    t(order),
    is_leaf(is_leaf),
    keys(max_key_num(), -1),
    values(max_key_num()),
    children(max_child_num())
    {}

template<typename K, typename V, typename Oit>
BTree<K, V, Oit>::BTreeNode::~BTreeNode() {
    if (!is_leaf) {
        for (int i = 0; i < used_keys + 1; ++i) {
            delete children[i];
        }
    }
    for (int i = 0; i < used_keys; ++i)
        delete values[i];
}

template<typename K, typename V, typename Oit>
typename BTree<K, V, Oit>::Node* BTree<K, V, Oit>::BTreeNode::find_node(const K &key) {
    int idx = find_key_pos(key);
    if (idx < used_keys && keys[idx] == key) {
        return this;
    }
    return is_leaf ? nullptr : children[idx]->find_node(key);
}

template<typename K, typename V, typename Oit>
int BTree<K, V, Oit>::BTreeNode::key_idx_in_node(const K &key) const {
    for (int i = 0; i < used_keys; ++i) {
        if (keys[i] == key)
            return i;
    }
    return used_keys;
}

template<typename K, typename V, typename Oit>
int BTree<K, V, Oit>::BTreeNode::find_key_pos(const K &key) const {
    int pos = 0;
    while (pos < used_keys && keys[pos] < key)
        ++pos;
    return pos;
}

template<typename K, typename V, typename Oit>
void BTree<K, V, Oit>::BTreeNode::set(int pos, const V &value) {
    delete values[pos]; // todo: think about deletion
    values[pos] = new V(value);
}

template<typename K, typename V, typename Oit>
void BTree<K, V, Oit>::BTreeNode::insert_non_full(const K &key, const V &value) {
    int pos = find_key_pos(key);
    if (is_leaf) {
        for (int i = used_keys; i > pos; --i) {
            keys[i] = keys[i - 1];
            values[i] = values[i - 1];
        }
        keys[pos] = key;
        values[pos] = new V(value);
        used_keys += 1;
    } else {
        if (children[pos]->is_full()) {
            split_child(pos, children[pos]);
            if (keys[pos] < key) {
                pos++;
            }
        }
        children[pos]->insert_non_full(key, value);
    }
}

template<typename K, typename V, typename Oit>
void BTree<K, V, Oit>::BTreeNode::split_child(const int &pos, Node *node) {
    // Create a new node to store (t-1) keys of divided node
    Node *new_node = new Node(node->t, node->is_leaf);
    new_node->used_keys = t - 1;

    // Copy the last (t-1) keys of divided node to new_node
    for (int i = 0; i < t - 1; ++i) {
        new_node->keys[i] = node->keys[i + t];
        new_node->values[i] = node->values[i + t];
    }
    // Copy the last (t-1) children of divided node to new_node
    if (!node->is_leaf) {
        for (int i = 0; i < t; ++i) {
            new_node->children[i] = node->children[i + t];
        }
    }
    // Reduce the number of keys in the divided node
    node->used_keys = t - 1;

    // Shift children, keys and values to right
    for (int i = used_keys + 1; i > pos + 1; --i) {
        children[i] = children[i - 1];
    }
    children[pos + 1] = new_node;

    for (int i = used_keys; i > pos; --i) {
        keys[i] = keys[i - 1];
        values[i] = values[i - 1];
    }
    // set the key-divider
    keys[pos] = node->keys[t - 1];
    values[pos] = node->values[t - 1];
    ++used_keys;
}

template<typename K, typename V, typename Oit>
void BTree<K, V, Oit>::BTreeNode::traverse() {
    int i;
    auto traverse_child = [&]() {
        if (!is_leaf) {
            cout << endl;
            children[i]->traverse();
            cout << endl;
        }
    };
    for (i = 0; i < used_keys; ++i) {
        traverse_child();
        cout << keys[i] << ": " << *values[i] << " | ";
    }
    traverse_child();
}



