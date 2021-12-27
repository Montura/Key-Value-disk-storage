#pragma once

#include "btree_node.h"
#include "btree.h"

template<class K, class V>
BTree<K,V>::BTreeNode::BTreeNode(const int& t, bool isLeaf) :
    used_keys(0),
    t(t),
    flag(isLeaf ? 1 : 0),
    arrayPosKey(max_key_num(), -1),
    arrayPosChild(max_child_num(), -1)
{}

template<class K, class V>
BTree<K,V>::BTreeNode::~BTreeNode() {}


template<class K, class V>
bool BTree<K,V>::BTreeNode::is_leaf() const {
    return (flag & 1) == 1;
}

template<class K, class V>
bool BTree<K,V>::BTreeNode::is_full() const {
    return used_keys == max_key_num();
}

template<class K, class V>
void BTree<K,V>::BTreeNode::split_child(BTree* bTree, const int index, Node& node) {
    Node new_node(node.t, node.is_leaf());
    new_node.used_keys = t - 1;
    for (int i = 0; i < t - 1; i++) {
        new_node.arrayPosKey[i] = node.arrayPosKey[i + t];
        node.arrayPosKey[i + t] = -1;
    }
    if (!node.is_leaf()) {
        for (int i = 0; i < t; i++) {
            new_node.arrayPosChild[i] = node.arrayPosChild[i + t];
            node.arrayPosChild[i + t] = -1;
        }
    }

    bTree->setPosEndFileWrite();
    new_node.m_pos = bTree->getPosFileWrite();
    //write new node
    bTree->write_node(new_node, new_node.m_pos);

    node.used_keys = t - 1;

    //write node
    bTree->write_node(node, node.m_pos);

    for (int i = used_keys; i >= index + 1; --i) {
        arrayPosChild[i + 1] = arrayPosChild[i];
    }
    arrayPosChild[index + 1] = new_node.m_pos;

    for (int i = used_keys - 1; i >= index; --i) {
        arrayPosKey[i + 1] = arrayPosKey[i];
    }

    arrayPosKey[index] = node.arrayPosKey[t - 1];
    used_keys += 1;

    //write node
    bTree->write_node(*this, m_pos);
}

template<class K, class V>
Entry<K, V> BTree<K,V>::BTreeNode::read_entry(BTree* bTree, const int i) {
    if (i < 0 || i > used_keys - 1) {
        return {}; // nullptr
    }

    int pos = arrayPosKey[i];
    Entry<K, V> entry;

    //read entry
    bTree->read_entry(entry, pos);
    return entry;
}

template<class K, class V>
typename BTree<K,V>::BTreeNode BTree<K,V>::BTreeNode::get_node(BTree* bTree, const int i) {
    if (i < 0 || i > used_keys) {
        return Node(1, false); // dummy
    }

    int pos = arrayPosChild[i];
    Node node(t, false);

    //read node
    bTree->read_node(&node, pos);
    return node;
}

template<class K, class V>
void BTree<K,V>::BTreeNode::insert_non_full(BTree* bTree, const Entry<K, V>& entry) {
    int i = used_keys - 1;
    Entry<K, V> entryTmp = read_entry(bTree, i);

    if (is_leaf()) {
        while (i >= 0 && entryTmp.key > entry.key) {
            arrayPosKey[i + 1] = arrayPosKey[i];
            i--;
            entryTmp = read_entry(bTree, i);
        }

        bTree->setPosEndFileWrite();
        int pos = bTree->getPosFileWrite();

        //write entry
        bTree->write_entry(entry, pos);

        arrayPosKey[i + 1] = pos;
        used_keys += 1;

        //write node
        bTree->write_node(*this, m_pos);
    } else {
        i = find_key_bin_search(bTree, entry.key);

        Node node = get_node(bTree, i);

        if (node.is_full()) {
            split_child(bTree, i, node);

            entryTmp = read_entry(bTree, i);

            if (entryTmp.key < entry.key) {
                i++;
            }
        }

        node = get_node(bTree, i);
        node.insert_non_full(bTree, entry);
    }
}

template<class K, class V>
void BTree<K,V>::BTreeNode::traverse(BTree* bTree) {
    int i;
    Node node;
    Entry<K, V> entry;
    for (i = 0; i < used_keys; ++i) {
        if (!is_leaf()) {
            node = get_node(bTree, i);
            cout << endl;
            node.traverse(bTree);
            cout << endl;
        }
        entry = read_entry(bTree, i);
        cout << "[key]: " << entry->key << " - [value]: " << entry->value << " ";
    }
    if (!is_leaf()) {
        node = get_node(bTree, i);
        cout << endl;
        node.traverse(bTree);
        cout << endl;
    }
}

template<class K, class V>
int BTree<K,V>::BTreeNode::find_key_bin_search(BTree*bTree, const K& key) {
    int low = 0;
    int hight = used_keys - 1;
    int middle = (low + hight) / 2;
    Entry<K, V> entry = read_entry(bTree, middle);

    while (low <= hight) {
        if (entry.key == key) {
            return middle;
        } else {
            if (entry.key > key) {
                hight = middle - 1;
            } else {
                low = middle + 1;
            }
        }
        middle = (low + hight) / 2;
        entry = read_entry(bTree, middle);
    }
    return hight + 1;
}


template<class K, class V>
Entry<K, V> BTree<K,V>::BTreeNode::find(BTree* bTree, const K& key) {
    int i = 0;
    Entry<K, V> entry = read_entry(bTree, i);
    while (i < used_keys && entry.key < key) {
        ++i;
        entry = read_entry(bTree, i);
    }

    if (i < used_keys && entry.key == key) {
        return entry;
    }

    if (is_leaf()) {
        return {}; // nullptr dummy
    }

    return get_node(bTree, i).find(bTree, key);
}

template<class K, class V>
bool BTree<K,V>::BTreeNode::set(BTree* bTree, const K& key, const V& value) {
    int i = find_key_bin_search(bTree, key);

    if (i < used_keys) {
        Entry<K, V> entry = read_entry(bTree, i);
        if (entry.key == key) {
            bTree->write_flag('0', arrayPosKey[i]);

            bTree->setPosEndFileWrite();
            int curr_pos = bTree->getPosFileWrite();

            arrayPosKey[i] = curr_pos;
            entry.value = value;

            bTree->write_entry(entry, curr_pos);
            bTree->write_node(*this, m_pos);

            return true;
        }
    }
    if (is_leaf()) {
        return false;
    }

    Node node = get_node(bTree, i);
    bool res = node.set(bTree, key, value);

    return res;
}

template<class K, class V>
bool BTree<K,V>::BTreeNode::remove(BTree* bTree, const K& key) {
    bool res;

    int index = find_key_bin_search(bTree, key);
    Entry<K, V> entry = read_entry(bTree, index);

    if (index < used_keys && entry.key == key) {
        res = is_leaf() ? remove_from_leaf(bTree, index) : remove_from_non_leaf(bTree, index);
        bTree->write_node(*this, m_pos);
    } else {
        if (is_leaf()) {
            return false;
        }

        bool flag = (index == used_keys);
        auto child = get_node(bTree, index);

        if (child.used_keys < t) {
            fill_node(bTree, index);
        }

        if (flag && index > used_keys) {
            auto childPrev = get_node(bTree, index - 1);
            childPrev.remove(bTree, key);

            childPrev = get_node(bTree, index - 1);
            if (childPrev.t != 1) {
                bTree->write_node(childPrev, childPrev.m_pos);
            }
        } else {
            auto m_child = get_node(bTree, index);
            m_child.remove(bTree, key);
        }

        //write node
        auto child_2 = get_node(bTree, index);
        if (child_2.t != 1) {
            bTree->write_node(child_2, child_2.m_pos);
        }
        res = true;
    }

    return res;
}

template<class K, class V>
bool BTree<K,V>::BTreeNode::remove_from_leaf(BTree* bTree, const int index) {
    bTree->write_flag('0', arrayPosKey[index]);

    for (int i = index + 1; i < used_keys; ++i) {
        arrayPosKey[i - 1] = arrayPosKey[i];
    }
    used_keys--;

    return true;
}

template<class K, class V>
bool BTree<K,V>::BTreeNode::remove_from_non_leaf(BTree* bTree, const int index) {
    Node node = get_node(bTree, index);
    Node nodeNext = get_node(bTree, index + 1);

    int curr_pos;
    bool res;

    if (node.used_keys >= t) {
        curr_pos = get_prev_entry_pos(bTree, index);
        arrayPosKey[index] = curr_pos;

        Entry<K, V> entry;
        bTree->read_entry(entry, curr_pos);
        K key = entry.key;

        res = node.remove(bTree, key);
    } else if (nodeNext.used_keys >= t) {
        curr_pos = get_next_entry_pos(bTree, index);
        arrayPosKey[index] = curr_pos;

        Entry<K, V> entry;
        bTree->read_entry(entry, curr_pos);
        K key = entry.key;

        res = nodeNext.remove(bTree, key);
    } else {
        Entry<K, V> entry = read_entry(bTree, index);
        merge_node(bTree, index);
        K key = entry.key;

        node = get_node(bTree, index);
        res = node.remove(bTree, key);
    }

    bTree->write_node(node, node.m_pos);

    return res;
}

template<class K, class V>
int BTree<K,V>::BTreeNode::get_prev_entry_pos(BTree* bTree, const int index) {
    Node nodeCurrent(t, false);
    bTree->read_node(&nodeCurrent, arrayPosChild[index]);
    while (!nodeCurrent.is_leaf()) {
        bTree->read_node(&nodeCurrent, nodeCurrent.arrayPosChild[nodeCurrent.used_keys]);
    }

    return nodeCurrent.arrayPosKey[nodeCurrent.used_keys - 1];
}

template<class K, class V>
int BTree<K,V>::BTreeNode::get_next_entry_pos(BTree* bTree, const int index) {
    Node nodeCurrent(t, false);
    bTree->read_node(&nodeCurrent, arrayPosChild[index + 1]);
    while (!nodeCurrent.is_leaf()) {
        bTree->read_node(&nodeCurrent, nodeCurrent.arrayPosChild[0]);
    }

    return nodeCurrent.arrayPosKey[0];
}

template<class K, class V>
void BTree<K,V>::BTreeNode::merge_node(BTree* bTree, const int index) {
    Node child = get_node(bTree, index);
    Node childNext = get_node(bTree, index + 1);

    child.arrayPosKey[t - 1] = arrayPosKey[index];

    for (int i = 0; i < childNext.used_keys; ++i) {
        child.arrayPosKey[i + t] = childNext.arrayPosKey[i];
    }
    if (!child.is_leaf()) {
        for (int i = 0; i <= childNext.used_keys; ++i) {
            child.arrayPosChild[i + t] = childNext.arrayPosChild[i];
        }
    }

    child.used_keys += childNext.used_keys + 1;

    // write node
    bTree->write_node(child, arrayPosChild[index]);

    childNext.flag = childNext.flag | (1 << 1);
    // write node
    bTree->write_node(childNext, arrayPosChild[index + 1]);

    for (int i = index + 1; i < used_keys; ++i) {
        arrayPosKey[i - 1] = arrayPosKey[i];
    }
    for (int i = index + 1; i < used_keys; ++i) {
        arrayPosChild[i] = arrayPosChild[i + 1];
    }

    used_keys--;
    bTree->write_node(*this, m_pos);
}

template<class K, class V>
void BTree<K,V>::BTreeNode::fill_node(BTree* bTree, const int index) {
    Node childPrev = get_node(bTree, index - 1);
    Node childNext = get_node(bTree, index + 1);

    if (index != 0 && childPrev.used_keys >= t) {
        borrow_from_node_prev(bTree, index);
    } else if (index != used_keys && childNext.used_keys >= t) {
        borrow_from_node_next(bTree, index);
    } else {
        if (index != used_keys) {
            merge_node(bTree, index);
        } else {
            merge_node(bTree, index - 1);
        }
    }

}

template<class K, class V>
void BTree<K,V>::BTreeNode::borrow_from_node_prev(BTree* bTree, const int index) {
    Node childPrev = get_node(bTree, index - 1); // nho delete
    Node child = get_node(bTree, index); // nho delete
    for (int i = child.used_keys - 1; i >= 0; --i) {
        child.arrayPosKey[i + 1] = child.arrayPosKey[i];
    }
    if (!child.is_leaf()) {
        for (int i = child.used_keys; i >= 0; --i) {
            child.arrayPosChild[i + 1] = child.arrayPosChild[i];
        }
    }

    child.arrayPosKey[0] = arrayPosKey[index - 1];
    if (!child.is_leaf()) {

        child.arrayPosChild[0] = childPrev.arrayPosChild[childPrev.used_keys];
    }
    arrayPosKey[index - 1] = childPrev.arrayPosKey[childPrev.used_keys - 1];
    child.used_keys++;
    childPrev.used_keys--;

    bTree->write_node(child, child.m_pos);
    bTree->write_node(childPrev, childPrev.m_pos);
    bTree->write_node(*this, m_pos);
}

template<class K, class V>
void BTree<K,V>::BTreeNode::borrow_from_node_next(BTree* bTree, const int index) {
    Node childNext = get_node(bTree, index + 1);
    Node child = get_node(bTree, index);

    child.arrayPosKey[child.used_keys] = arrayPosKey[index];

    if (!child.is_leaf()) {
        child.arrayPosChild[child.used_keys + 1] = childNext.arrayPosChild[0];
    }

    arrayPosKey[index] = childNext.arrayPosKey[0];
    for (int i = 1; i < childNext.used_keys; ++i) {
        childNext.arrayPosKey[i - 1] = childNext.arrayPosKey[i];
    }
    if (!childNext.is_leaf()) {
        for (int i = 1; i <= childNext.used_keys; ++i) {
            childNext.arrayPosChild[i - 1] = childNext.arrayPosChild[i];
        }
    }
    child.used_keys++;
    childNext.used_keys--;

    bTree->write_node(child, child.m_pos);
    bTree->write_node(childNext, childNext.m_pos);
    bTree->write_node(*this, m_pos);
}


