#pragma once

#include "btree.h"

template<typename K, typename V, int t, typename Oit>
BTree<K, V, t, Oit>::BTreeNode::BTreeNode(bool is_leaf):
    is_leaf(is_leaf),
    keys(max_key_num, -1),
    children(max_child_num),
    values(max_key_num) {}

template<typename K, typename V, int t, typename Oit>
BTree<K, V, t, Oit>::BTreeNode::~BTreeNode() {
    if (!is_leaf) {
        for (int i = 0; i < used_keys + 1; ++i) {
            delete children[i];
        }
    } else {
        for (int i = 0; i < used_keys; ++i)
            delete values[i];
    }
}

template<typename K, typename V, int t, typename Oit>
int BTree<K, V, t, Oit>::BTreeNode::key_pos_in_leaf_node(const K &key) const {
    assert(is_leaf);
    for (int i = 0; i < used_keys; ++i) {
        if (keys[i] == key)
            return i;
    }
    return used_keys;
}

template<typename K, typename V, int t, typename Oit>
bool BTree<K, V, t, Oit>::BTreeNode::has_key(const K &key) const {
    return key_pos_in_leaf_node(key) != used_keys;
}

template<typename K, typename V, int t, typename Oit>
int BTree<K, V, t, Oit>::BTreeNode::find_key_pos(const K &key) const {
    int pos = 0;
    while (pos < used_keys && keys[pos] < key)
        ++pos;
    return pos;
}

template<typename K, typename V, int t, typename Oit>
void BTree<K, V, t, Oit>::BTreeNode::set(int pos, const V &value) {
    delete values[pos]; // todo: think about deletion
    values[pos] = new V(value);
}

template<typename K, typename V, int t, typename Oit>
void BTree<K, V, t, Oit>::BTreeNode::insert_non_full(const K &key, const V &value) {
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
        if (children[pos]->used_keys == max_key_num) {
            split_child(pos, children[pos]);
            if (keys[pos] < key) {
                pos++;
            }
        }
        children[pos]->insert_non_full(key, value);
    }
}

template<typename K, typename V, int t, typename Oit>
void BTree<K, V, t, Oit>::BTreeNode::split_child(const int &pos, Node *node) {
    // Create a new node to store (t-1) keys of divided node
    Node *new_node = new Node(node->is_leaf);
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
    node->used_keys = t;

    // Shift children, keys and values to right
    for (int i = used_keys + 1; i >= pos + 2; --i) {
        children[i] = children[i - 1];
    }
    children[pos + 1] = new_node;

    for (int i = used_keys; i >= pos + 1; --i) {
        keys[i] = keys[i - 1];
        values[i] = values[i - 1];
    }
    // set the key-divider
    keys[pos] = node->keys[t - 1];
    ++used_keys;
}

template<typename K, typename V, int t, typename Oit>
void BTree<K, V, t, Oit>::BTreeNode::traverse() {
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
        cout << keys[i] << " ";
        if (is_leaf) {
            cout << ": " << *values[i] << " | ";
        }
    }
    traverse_child();
}



