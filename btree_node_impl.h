#pragma once

#include "btree.h"

//static int ctors = 0;
//static int dtros = 0;
//static int total_deleted = 0;
//static int total_values = 0;

template<typename K, typename V, typename Oit>
BTree<K, V, Oit>::BTreeNode::BTreeNode(int order, bool is_leaf):
    t(order),
    is_leaf(is_leaf),
    keys(max_key_num(), -1),
    values(max_key_num(), nullptr),
    children(max_child_num(), nullptr)
    {
//        std::cout << "BTreeNode ctors: " << ++ctors << endl;
    }


template<typename K, typename V, typename Oit>
BTree<K, V, Oit>::BTreeNode::~BTreeNode() {
//    std::cout << "BTreeNode dtors: " << ++dtros << endl;
    if (!is_deleted) {
        for (int i = 0; i < used_keys; ++i) {
//            std::cout << "value dtors: " << ++total_deleted << endl;
            delete values[i];
        }
        for (int i = 0; i <= used_keys; ++i) {
            delete children[i];
        }
    }
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
//        cout << "values created: " << ++total_values << endl;
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
        node->keys[i + t] = -1;
        node->values[i + t] = nullptr;
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
bool BTree<K, V, Oit>::BTreeNode::remove(const K &key) {
    int pos = find_key_pos(key);
//    cout << "pos: " << pos << endl;
    bool success;
    if (pos < used_keys && (keys[pos] == key)) {
        success = is_leaf ? remove_from_leaf(pos) : remove_from_non_leaf(pos);
    } else {
        if (is_leaf) {
            return false;
        }
        // If the child where the key is supposed to exist has less that t keys, we fill that child
        if (children[pos]->used_keys < t) {
            fill_or_merge_node(pos);
        }

        // If the last child has been merged, it must have merged with the previous child and
        //      so we recurse on the (pos-1)-th child.
        // Else, we recurse on the (pos)-th child which now has at least t keys
        int child_idx = (pos > used_keys) ? (pos - 1) : pos;
        success = children[child_idx]->remove(key);
    }
    return success;
}

template<typename K, typename V, typename Oit>
bool BTree<K, V, Oit>::BTreeNode::remove_from_leaf(const int pos) {
//    std::cout << "value dtors: " << ++total_deleted << endl;
    delete values[pos];
    // shift to the left by 1 all the keys after the pos
    for (int i = pos + 1; i < used_keys; ++i) {
        keys[i - 1] = keys[i];
        values[i - 1] = values[i];
    }
    --used_keys;
    return true;
}

template<typename K, typename V, typename Oit>
bool BTree<K, V, Oit>::BTreeNode::remove_from_non_leaf(const int pos) {
    // 1. If the child[pos] has >= T keys, find the PREVIOUS in the subtree rooted at child[pos].
    // 2. Replace keys[pos], values[pos] by the PREVIOUS[key|value].
    // 3. Recursively delete PREVIOUS in child[pos].
    if (children[pos]->used_keys >= t) {
        auto [key, value] = get_previous_key_value(pos);
        keys[pos] = key;
        values[pos] = value;
        return children[pos]->remove(key);
    }
    // If the child[pos] has <= T keys, check the child[pos + 1].
    // 1. If child[pos + 1] has >= T keys, find the NEXT in the subtree rooted at child[pos + 1].
    // 2. Replace keys[pos], values[pos] by the NEXT[key|value].
    // 3. Recursively delete NEXT in child[pos + 1].
    else if (children[pos + 1]->used_keys >= t) {
        auto [key, value] = get_next_key_value(pos);
        keys[pos] = key;
        values[pos] = value;
        return children[pos + 1]->remove(key);
    // 1. Now child[pos] and child[pos + 1] has < T keys.
    // 2. Merge key and child[pos + 1] into child[pos].
    // 3. Now child[pos] has (2 * t - 1) keys
    // 4. Recursively delete KEY from child[pos]
    } else {
        int key = keys[pos];
        merge_node(pos);
        return children[pos]->remove(key);
    }
}

template<typename K, typename V, typename Oit>
std::pair<K,V*> BTree<K, V, Oit>::BTreeNode::get_previous_key_value(const int pos) const {
    Node* curr = children[pos];
    // Keep moving to the right most node until CURR becomes a leaf
    while (!curr->is_leaf) {
        curr = curr->children[curr->used_keys];
    }
    int last_key_pos = curr->used_keys - 1;
    return std::make_pair(curr->keys[last_key_pos], curr->values[last_key_pos]);
}

template<typename K, typename V, typename Oit>
std::pair<K,V*> BTree<K, V, Oit>::BTreeNode::get_next_key_value(const int pos) const {
    Node* curr = children[pos + 1];
    // Keep moving the left most node until CURR becomes a leaf
    while (!curr->is_leaf) {
        curr = curr->children[0];
    }
    return std::make_pair(curr->keys[0], curr->values[0]);
}

template<typename K, typename V, typename Oit>
void BTree<K, V, Oit>::BTreeNode::merge_node(const int pos) {
    Node* curr = children[pos];
    Node* next = children[pos + 1];

    curr->keys[t - 1] = keys[pos];
    curr->values[t - 1] = values[pos];

    // Copy all from child[pos + 1] to child[pos]
    for (int i = 0; i < next->used_keys; ++i) {
        curr->keys[i + t] = next->keys[i];
        curr->values[i + t] = next->values[i];
    }
    // Copy the children from child[pos + 1] to child[pos]
    if (!curr->is_leaf) {
        for (int i = 0; i <= next->used_keys; ++i) {
            curr->children[i + t] = next->children[i];
        }
    }
    for (int i = pos + 1; i < used_keys; ++i) {
        keys[i - 1] = keys[i];
        values[i - 1] = values[i];
    }
    for (int i = pos + 1; i < used_keys; ++i) {
        children[i] = children[i + 1];
    }
    curr->used_keys += next->used_keys + 1;
    --used_keys;
    next->is_deleted = true;
    delete next;
}

template<typename K, typename V, typename Oit>
void BTree<K, V, Oit>::BTreeNode::fill_or_merge_node(const int pos) {
    // If the left child has >= (T - 1) keys, borrow a key from it
    if (pos != 0 && children[pos - 1]->used_keys >= t) {
        borrow_from_node_prev(pos);

    // If the right child has >= (T - 1) keys, borrow a key from it
    } else if (pos != used_keys && children[pos + 1]->used_keys >= t) {
        borrow_from_node_next(pos);

    // Merge child[pos] with its sibling
    // If child[pos] is the last child, merge it with its previous sibling
    // Otherwise merge it with its next sibling
    } else {
        int idx = (pos != used_keys) ? pos : pos - 1;
        merge_node(idx);
    }
}

template<typename K, typename V, typename Oit>
void BTree<K, V, Oit>::BTreeNode::borrow_from_node_prev(const int pos) {
    Node* curr = children[pos];
    Node* prev = children[pos - 1];
    for (int i = curr->used_keys - 1; i >= 0; --i) {
        curr->keys[i + 1] = curr->keys[i];
        curr->values[i + 1] = curr->values[i];
    }
    if (!curr->is_leaf) {
        for (int i = curr->used_keys; i >= 0; --i) {
            curr->children[i + 1] = curr->children[i];
        }
    }
    curr->keys[0] = keys[pos - 1];
    curr->values[0] = values[pos - 1];

    if (!curr->is_leaf) {
        curr->children[0] = prev->children[prev->used_keys];
    }
    keys[pos - 1] = prev->keys[prev->used_keys - 1];
    values[pos - 1] = prev->values[prev->used_keys - 1];

    curr->used_keys++;
    prev->used_keys--;
}

template<typename K, typename V, typename Oit>
void BTree<K, V, Oit>::BTreeNode::borrow_from_node_next(const int pos) {
    Node* child = children[pos];
    Node* child_next = children[pos + 1];

    child->keys[child->used_keys] = keys[pos];
    child->values[child->used_keys] = values[pos];

    if (!child->is_leaf) {
        child->children[child->used_keys + 1] = child_next->children[0];
    }
    keys[pos] = child_next->keys[0];
    values[pos] = child_next->values[0];

    for (int i = 1; i < child_next->used_keys; ++i) {
        child_next->keys[i - 1] = child_next->keys[i];
        child_next->values[i - 1] = child_next->values[i];
    }
    if (!child_next->is_leaf) {
        for (int i = 1; i <= child_next->used_keys; ++i) {
            child_next->children[i - 1] = child_next->children[i];
        }
    }
    child->used_keys++;
    child_next->used_keys--;
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



