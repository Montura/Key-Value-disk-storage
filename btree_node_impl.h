#pragma once

#include "btree.h"


template<typename K, typename V, typename Oit>
BTree<K, V, Oit>::BTreeNode::BTreeNode(int order, bool is_leaf):
    t(order),
//    is_leaf(is_leaf),
    keys(max_key_num(), -1),
//    values(max_key_num(), nullptr),
//    children(max_child_num(), nullptr),
    children_idx(max_child_num(), -1)
    {
//        std::cout << "BTreeNode ctors: " << ++ctors << endl;
    }


template<typename K, typename V, typename Oit>
BTree<K, V, Oit>::BTreeNode::~BTreeNode() {
//    if (!is_deleted) {
        for (int i = 0; i < used_keys; ++i) {
//            delete values[i];
        }
        for (int i = 0; i <= used_keys; ++i) {
//            delete children[i];
        }
//    }
}

//template<typename K, typename V, typename Oit>
//typename BTree<K, V, Oit>::Node* BTree<K, V, Oit>::BTreeNode::find_node(const K &key) {
//    int idx = find_key_pos(key);
//    if (idx < used_keys && keys[idx] == key) {
//        return this;
//    }
//    return is_leaf ? nullptr : children[idx]->find_node(key);
//}

//template<typename K, typename V, typename Oit>
//int BTree<K, V, Oit>::BTreeNode::key_idx_in_node(const K &key) const {
//    for (int i = 0; i < used_keys; ++i) {
//        if (keys[i] == key)
//            return i;
//    }
//    return used_keys;
//}
//
//template<typename K, typename V, typename Oit>
//int BTree<K, V, Oit>::BTreeNode::find_key_pos(const K &key) const {
//    int pos = 0;
//    while (pos < used_keys && keys[pos] < key)
//        ++pos;
//    return pos;
//}

template <typename K, typename V, typename Oit>
std::optional<std::pair<K, V>> BTree<K, V, Oit>::BTreeNode::search(BTree* tree, const K& key) {
    int i = find_key_binary_search(tree, key);

    if (i < used_keys) {
        auto optional = get_key_value(tree, i);
        assert(optional.has_value());
        auto pair = optional.value();
        if (pair.first == key) {
            return pair;
        }
    }

    if (checkIsLeaf()) {
        return {};
    }

    Node* node = getBTreeNodeStore(tree, i);
    auto optional = node->search(tree, key);

    delete node;
    return optional;
}


template<typename K, typename V, typename Oit>
bool BTree<K, V, Oit>::BTreeNode::set(BTree* tree, const K& key , const V &value) {
    int i = find_key_binary_search(tree, key);

    Node* node;
    if (i < used_keys) {
        auto optional = get_key_value(tree, i);
        assert(optional.has_value());
        auto [tmp_key, tmp_value] = optional.value();
        if (tmp_key == key) {
            tree->writeFlag('0', keys[i]);

            tree->setPosEndFileWrite();
            int f_pos = tree->getPosFileWrite();

            keys[i] = f_pos;

            tree->write_key_value(f_pos, key, value);

            tree->writeNode(this, pos);

            return true;
        }
    }
    if (checkIsLeaf()) {
        return false;
    }

    node = getBTreeNodeStore(tree, i);
    bool success = node->set(tree, key, value);

    delete node;
    return success;
}

template<typename K, typename V, typename Oit>
void BTree<K, V, Oit>::BTreeNode::insert_non_full(BTree* tree, const K &key, const V &value) {
    int i = used_keys - 1;
    K tmp_key;
    V tmp_value;
    auto optional = get_key_value(tree, i);
    assert(optional.has_value());
    std::tie(tmp_key, tmp_value) = optional.value();

    if (checkIsLeaf()) {
        while (i >= 0 && tmp_key > key) {
            keys[i + 1] = keys[i];
            i--;
            auto optional = get_key_value(tree, i);
            assert(optional.has_value());
            std::tie(tmp_key, tmp_value) = optional.value();
        }

        tree->setPosEndFileWrite();
        int pos = tree->getPosFileWrite();

        // write entry
        tree->write_key_value(pos, key, value);

        keys[i + 1] = pos;
        ++used_keys;

        // write node
        tree->writeNode(this, pos);
    } else {
        int i = find_key_binary_search(tree, key);

        Node* node = getBTreeNodeStore(tree, i);

        if (node->is_full()) {
            split_child(tree, i, node);
            auto optional = get_key_value(tree, i);
            assert(optional.has_value());
            std::tie(tmp_key, tmp_value) = optional.value();
            if (tmp_key < key) {
                i++;
            }

            delete node;
        }

        node = getBTreeNodeStore(tree, i);
        node->insert_non_full(tree, key, value);

        delete node;
    }
}

template<typename K, typename V, typename Oit>
void BTree<K, V, Oit>::BTreeNode::split_child(BTree* tree, const int &idx, Node *node) {
    // Create a new node to store (t-1) keys of divided node
    Node *new_node = new Node(node->t, checkIsLeaf());
    new_node->used_keys = t - 1;

    // Copy the last (t-1) keys of divided node to new_node
    for (int i = 0; i < t - 1; ++i) {
        new_node->keys[i] = node->keys[i + t];
        node->keys[i + t] = -1;
    }
    // Copy the last (t-1) children of divided node to new_node
    if (!checkIsLeaf()) {
        for (int i = 0; i < t; ++i) {
            new_node->children_idx[i] = node->children_idx[i + t];
            node->children_idx[i] = -1;
        }
    }

    tree->setPosEndFileWrite();
//    new_node->pos = tree->getPosFileWrite();
    //write new node
    tree->writeNode(new_node, new_node->pos);

    // Reduce the number of keys in the divided node
    node->used_keys = t - 1;

    //write node
    tree->writeNode(node, node->pos);

    // Shift children, keys and values to right
    for (int i = used_keys + 1; i > idx + 1; --i) {
        children_idx[i] = children_idx[i - 1];
    }
    children_idx[idx + 1] = new_node->pos;

    for (int i = used_keys; i > idx; --i) {
        keys[i] = keys[i - 1];
//        values[i] = values[i - 1];
    }
    // set the key-divider
    keys[idx] = node->keys[t - 1];
//    values[idx] = node->values[t - 1];
    ++used_keys;

    //write node
    tree->writeNode(this, pos);
    delete new_node;
}

template<typename K, typename V, typename Oit>
bool BTree<K, V, Oit>::BTreeNode::remove(BTree* tree, const K &key) {
    int idx = find_key_binary_search(tree, key);
    bool success;
    K tmp_key;
    K tmp_value;

    auto optional = get_key_value(tree, idx);
    assert(optional.has_value());
    std::tie(tmp_key, tmp_value) = optional.value();


    if (idx < used_keys && (tmp_key == key)) {
        success = checkIsLeaf() ? remove_from_leaf(tree, idx) : remove_from_non_leaf(tree, idx);
        //write node;
        tree->writeNode(this, pos);
    } else {
        if (checkIsLeaf()) {
            return false;
        }
        Node* node = getBTreeNodeStore(tree, idx);

        // If the child where the key is supposed to exist has less that t keys, we fill that child
        if (node->used_keys < t) {
            fill_or_merge_node(tree, idx);
            node = getBTreeNodeStore(tree, idx);
        }

        // If the last child has been merged, it must have merged with the previous child and
        //      so we recurse on the (pos-1)-th child.
        // Else, we recurse on the (pos)-th child which now has at least t keys
        if (idx > used_keys) {
            Node* nodePrev = getBTreeNodeStore(tree, idx - 1);
            success = nodePrev->remove(tree, key);

            // write node
            tree->writeNode(nodePrev, nodePrev->pos);
            delete nodePrev;
        } else {
            // write node
            success = node->remove(tree, key);
            tree->writeNode(node, node->pos);
        }

        delete node;
    }
    return success;
}

template<typename K, typename V, typename Oit>
bool BTree<K, V, Oit>::BTreeNode::remove_from_leaf(BTree* tree, const int idx) {
    tree->writeFlag('0', keys[idx]);

    // shift to the left by 1 all the keys after the pos
    for (int i = idx + 1; i < used_keys; ++i) {
        keys[i - 1] = keys[i];
    }
    --used_keys;
    return true;
}

template<typename K, typename V, typename Oit>
bool BTree<K, V, Oit>::BTreeNode::remove_from_non_leaf(BTree* tree, const int idx) {
    auto node = getBTreeNodeStore(tree, idx);
    auto nodeNext = getBTreeNodeStore(tree, idx + 1);

    bool success = false;
    // 1. If the child[idx] has >= T keys, find the PREVIOUS in the subtree rooted at child[idx].
    // 2. Replace keys[idx], values[idx] by the PREVIOUS[key|value].
    // 3. Recursively delete PREVIOUS in child[idx].
    if (node->used_keys >= t) {
        int pos = get_previous_key_value(tree, idx);
        keys[idx] = pos;

        auto [key, value] = tree->read_key_value(pos);
        success = node->remove(tree, key);
    }
    // If the child[idx] has <= T keys, check the child[idx + 1].
    // 1. If child[idx + 1] has >= T keys, find the NEXT in the subtree rooted at child[idx + 1].
    // 2. Replace keys[idx], values[idx] by the NEXT[key|value].
    // 3. Recursively delete NEXT in child[idx + 1].
    else if (nodeNext->used_keys >= t) {
        int pos = get_next_key_value(tree, idx);
        keys[idx] = pos;

        auto [key, value] = tree->read_key_value(pos);
        success = nodeNext->remove(tree, key);
    // 1. Now child[idx] and child[idx + 1] has < T keys.
    // 2. Merge key and child[idx + 1] into child[idx].
    // 3. Now child[idx] has (2 * t - 1) keys
    // 4. Recursively delete KEY from child[idx]
    } else {
        merge_node(tree, idx);
        auto [key, value] = get_key_value(tree, idx);

        delete node;
        node = getBTreeNodeStore(tree, idx);
        success = node->remove(tree, key);
        node = nullptr;
    }
    delete node;
    delete nodeNext;

    return success;
}

template<typename K, typename V, typename Oit>
int BTree<K, V, Oit>::BTreeNode::get_previous_key_value(BTree* tree, const int idx) const {
    Node* curr = new Node(t, false);
    tree->readNode(curr, children_idx[idx]);
    // Keep moving to the right most node until CURR becomes a leaf
    while (!curr->is_leaf) {
        tree->readNode(curr, curr->children_idx[curr->used_keys]);
    }
    int last_key_pos = curr->keys[curr->used_keys - 1];
    delete curr;
    return last_key_pos;
}

template<typename K, typename V, typename Oit>
int BTree<K, V, Oit>::BTreeNode::get_next_key_value(BTree* tree, const int idx) const {
    Node* curr = new Node(t, false);
    tree->readNode(curr, children_idx[idx + 1]);
    // Keep moving the left most node until CURR becomes a leaf
    while (!curr->is_leaf) {
        tree->readNode(curr, curr->children_idx[0]);
    }
    int last_key_pos = curr->keys[0];
    delete curr;
    return last_key_pos;
}

template<typename K, typename V, typename Oit>
void BTree<K, V, Oit>::BTreeNode::merge_node(BTree* tree, const int idx) {
    Node* curr = getBTreeNodeStore(tree, idx);
    Node* next = getBTreeNodeStore(tree, idx + 1);

    curr->keys[t - 1] = keys[idx];

    // Copy all from child[idx + 1] to child[idx]
    for (int i = 0; i < next->used_keys; ++i) {
        curr->keys[i + t] = next->keys[i];
    }

    // Copy the children from child[idx + 1] to child[idx]
    if (!curr->is_leaf) {
        for (int i = 0; i <= next->used_keys; ++i) {
            curr->children_idx[i + t] = next->children_idx[i];
        }
    }
    curr->used_keys += next->used_keys + 1;

    //write node
    tree->writeNode(curr, children_idx[idx]);

    next->flag = next->flag | (1 << 1);

    //write node
    tree->writeNode(next, children_idx[idx + 1]);

    for (int i = idx + 1; i < used_keys; ++i) {
        keys[i - 1] = keys[i];
    }
    for (int i = idx + 1; i < used_keys; ++i) {
        children_idx[i] = children_idx[i + 1];
    }

    --used_keys;
    tree->writeNode(this, this->pos);

    delete curr;
    delete next;
}

template<typename K, typename V, typename Oit>
void BTree<K, V, Oit>::BTreeNode::fill_or_merge_node(BTree* tree, const int idx) {
    Node* prev = getBTreeNodeStore(tree, idx - 1);
    Node* next = getBTreeNodeStore(tree, idx + 1);

    // If the left child has >= (T - 1) keys, borrow a key from it
    if (idx != 0 && prev->used_keys >= t) {
        borrow_from_node_prev(tree, idx);

    // If the right child has >= (T - 1) keys, borrow a key from it
    } else if (idx != used_keys && next->used_keys >= t) {
        borrow_from_node_next(tree, idx);

    // Merge child[idx] with its sibling
    // If child[idx] is the last child, merge it with its previous sibling
    // Otherwise merge it with its next sibling
    } else {
        int new_idx = (idx != used_keys) ? idx : idx - 1;
        merge_node(tree, new_idx);
    }
    delete prev;
    delete next;
}

template<typename K, typename V, typename Oit>
void BTree<K, V, Oit>::BTreeNode::borrow_from_node_prev(BTree* tree, const int idx) {
    Node* prev = getBTreeNodeStore(tree, idx - 1);
    Node* curr = getBTreeNodeStore(tree, idx);

    for (int i = curr->used_keys - 1; i >= 0; --i) {
        curr->keys[i + 1] = curr->keys[i];
    }
    if (!curr->is_leaf) {
        for (int i = curr->used_keys; i >= 0; --i) {
            curr->children_idx[i + 1] = curr->children_idx[i];
        }
    }

    curr->keys[0] = keys[idx - 1];

    if (!curr->is_leaf) {
        curr->children_idx[0] = prev->children_idx[prev->used_keys];
    }
    keys[pos - 1] = prev->keys[prev->used_keys - 1];

    curr->used_keys++;
    prev->used_keys--;

    tree->writeNode(curr, curr->pos);
    tree->writeNode(prev, prev->pos);
    tree->writeNode(this, this->pos);

    delete prev;
    delete curr;
}

template<typename K, typename V, typename Oit>
void BTree<K, V, Oit>::BTreeNode::borrow_from_node_next(BTree* tree, const int idx) {
    Node* curr = getBTreeNodeStore(tree, idx);
    Node* next = getBTreeNodeStore(tree, idx + 1);

    curr->keys[curr->used_keys] = keys[idx];

    if (!curr->is_leaf) {
        curr->children_idx[curr->used_keys + 1] = next->children_idx[0];
    }

    keys[idx] = next->keys[0];
    for (int i = 1; i < next->used_keys; ++i) {
        next->keys[i - 1] = next->keys[i];
    }
    if (!next->is_leaf) {
        for (int i = 1; i <= next->used_keys; ++i) {
            next->children_idx[i - 1] = next->children_idx[i];
        }
    }
    curr->used_keys++;
    next->used_keys--;

    tree->writeNode(curr, curr->pos);
    tree->writeNode(next, next->pos);
    tree->writeNode(this, this->pos);

    delete next;
    delete curr;
}

template<typename K, typename V, typename Oit>
void BTree<K, V, Oit>::BTreeNode::traverse(BTree* tree) {
    int i;
    K key;
    V value;
    auto traverse_child = [&]() {
        if (!checkIsLeaf()) {
            Node *node = getBTreeNodeStore(tree, i);
            cout << endl;
            node->traverse(tree);
            cout << endl;
            delete node;
        }
    };

    for (i = 0; i < used_keys; ++i) {
        traverse_child();
        std::tie(key, value) = get_key_value(tree, i);
        cout << "[key]: " << key << " - [value]: " << value << " ";
    }
    traverse_child();
}


template <typename K, typename V, typename Oit>
std::optional<std::pair<K,V>> BTree<K, V, Oit>::BTreeNode::get_key_value(BTree* bTree, const int idx) {
    if (idx < 0 || idx > used_keys - 1) {
        return {};
    }

    int pos = keys[idx];
    return bTree->read_key_value(pos);
}

template <typename K, typename V, typename Oit>
int BTree<K, V, Oit>::BTreeNode::find_key_binary_search(BTree *tree, const K& key) {
    int low = 0;
    int hight = used_keys - 1;
    int middle = (low + hight) / 2;

    K tmp_key;
    V tmp_value;
    auto optional = get_key_value(tree, middle);
    assert(optional.has_value());
    std::tie(tmp_key, tmp_value) = optional.value();
    while (low <= hight) {
        if (tmp_key == key) {
            return middle;
        } else if (tmp_key > key) {
            hight = middle - 1;
        } else {
            low = middle + 1;
        }
        middle = (low + hight) / 2;
        auto optional = get_key_value(tree, middle);
        assert(optional.has_value());
        std::tie(tmp_key, tmp_value) = optional.value();
    }
    return hight + 1;
}

template <typename K, typename V, typename Oit>
typename BTree<K, V, Oit>::BTreeNode* BTree<K, V, Oit>::BTreeNode::getBTreeNodeStore(BTree* bTree, const int& i) {
    if (i < 0 || i > used_keys) {
        return nullptr;
    }

    int pos = children_idx[i];
    Node* node = new Node(t, false);

    // read node
    bTree->readNode(node, pos);
    return node;
}