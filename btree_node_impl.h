#pragma once

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
bool BTree<K,V>::BTreeNode::is_leaf() const {
    return (flag & 1) == 1;
}

template<class K, class V>
bool BTree<K,V>::BTreeNode::is_full() const {
    return used_keys == max_key_num();
}

template<class K, class V>
void BTree<K,V>::BTreeNode::split_child(BTree* bTree, const int index, Node& curr_node) {
    Node new_node(curr_node.t, curr_node.is_leaf());
    new_node.used_keys = t - 1;
    for (int i = 0; i < t - 1; i++) {
        new_node.arrayPosKey[i] = curr_node.arrayPosKey[i + t];
        curr_node.arrayPosKey[i + t] = -1;
    }
    if (!curr_node.is_leaf()) {
        for (int i = 0; i < t; i++) {
            new_node.arrayPosChild[i] = curr_node.arrayPosChild[i + t];
            curr_node.arrayPosChild[i + t] = -1;
        }
    }

    bTree->setPosEndFileWrite();
    new_node.m_pos = bTree->getPosFileWrite();
    //write new node
    bTree->write_node(new_node, new_node.m_pos);

    curr_node.used_keys = t - 1;

    // write current node
    bTree->write_node(curr_node, curr_node.m_pos);

    for (int i = used_keys; i >= index + 1; --i) {
        arrayPosChild[i + 1] = arrayPosChild[i];
    }
    arrayPosChild[index + 1] = new_node.m_pos;

    for (int i = used_keys - 1; i >= index; --i) {
        arrayPosKey[i + 1] = arrayPosKey[i];
    }

    arrayPosKey[index] = curr_node.arrayPosKey[t - 1];
    used_keys += 1;

    // write node
    bTree->write_node(*this, m_pos);
}

template<class K, class V>
Entry<K, V> BTree<K,V>::BTreeNode::read_entry(BTree* bTree, const int i) {
    if (i < 0 || i > used_keys - 1) {
        return {}; // nullptr
    }

    int pos = arrayPosKey[i];
    EntryT entry;
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
void BTree<K,V>::BTreeNode::insert_non_full(BTree* bTree, const EntryT& entry) {
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
    EntryT entry;
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
    EntryT entry = read_entry(bTree, middle);

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
    EntryT entry = read_entry(bTree, i);
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
    int idx = find_key_bin_search(bTree, key);

    if (idx < used_keys) {
        EntryT entry = read_entry(bTree, idx);
        if (entry.key == key) {
            bTree->write_flag('0', arrayPosKey[idx]);

            bTree->setPosEndFileWrite();
            int curr_pos = bTree->getPosFileWrite();

            arrayPosKey[idx] = curr_pos;
            entry.value = value;

            bTree->write_entry(entry, curr_pos);
            bTree->write_node(*this, m_pos);

            return true;
        }
    }
    if (is_leaf()) {
        return false;
    }

    Node node = get_node(bTree, idx);

    return node.set(bTree, key, value);
}

template<class K, class V>
bool BTree<K,V>::BTreeNode::remove(BTree* bTree, const K& key) {
    bool res;

    int idx = find_key_bin_search(bTree, key);
    EntryT entry = read_entry(bTree, idx);

    if (idx < used_keys && entry.key == key) {
        res = is_leaf() ? remove_from_leaf(bTree, idx) : remove_from_non_leaf(bTree, idx);
        bTree->write_node(*this, m_pos);
    } else {
        if (is_leaf()) {
            return false;
        }

        auto child = get_node(bTree, idx);

        if (child.used_keys < t) {
            fill_node(bTree, idx);
        }

        int child_idx = (idx > used_keys) ? (idx - 1) : idx;
        auto m_child  = get_node(bTree, child_idx);

        if (m_child.t != 1) {
            res =  m_child.remove(bTree, key);
            bTree->write_node(m_child, m_child.m_pos);
        }
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
    Node curr = get_node(bTree, index);
    Node next = get_node(bTree, index + 1);

    int curr_pos;
    bool res;

    if (curr.used_keys >= t) {
        curr_pos = get_prev_entry_pos(bTree, index);
        arrayPosKey[index] = curr_pos;

        EntryT entry;
        bTree->read_entry(entry, curr_pos);
        K key = entry.key;

        res = curr.remove(bTree, key);
    } else if (next.used_keys >= t) {
        curr_pos = get_next_entry_pos(bTree, index);
        arrayPosKey[index] = curr_pos;

        EntryT entry;
        bTree->read_entry(entry, curr_pos);
        K key = entry.key;

        res = next.remove(bTree, key);
    } else {
        EntryT entry = read_entry(bTree, index);
        merge_node(bTree, index);
        K key = entry.key;

        curr = get_node(bTree, index);
        res = curr.remove(bTree, key);
    }

    bTree->write_node(curr, curr.m_pos);

    return res;
}

template<class K, class V>
int BTree<K,V>::BTreeNode::get_prev_entry_pos(BTree* bTree, const int index) {
    Node curr(t, false);
    bTree->read_node(&curr, arrayPosChild[index]);
    while (!curr.is_leaf()) {
        bTree->read_node(&curr, curr.arrayPosChild[curr.used_keys]);
    }

    return curr.arrayPosKey[curr.used_keys - 1];
}

template<class K, class V>
int BTree<K,V>::BTreeNode::get_next_entry_pos(BTree* bTree, const int index) {
    Node curr(t, false);
    bTree->read_node(&curr, arrayPosChild[index + 1]);
    while (!curr.is_leaf()) {
        bTree->read_node(&curr, curr.arrayPosChild[0]);
    }

    return curr.arrayPosKey[0];
}

template<class K, class V>
void BTree<K,V>::BTreeNode::merge_node(BTree* bTree, const int index) {
    Node curr = get_node(bTree, index);
    Node next = get_node(bTree, index + 1);

    curr.arrayPosKey[t - 1] = arrayPosKey[index];

    for (int i = 0; i < next.used_keys; ++i) {
        curr.arrayPosKey[i + t] = next.arrayPosKey[i];
    }
    if (!curr.is_leaf()) {
        for (int i = 0; i <= next.used_keys; ++i) {
            curr.arrayPosChild[i + t] = next.arrayPosChild[i];
        }
    }

    curr.used_keys += next.used_keys + 1;

    // write node
    bTree->write_node(curr, arrayPosChild[index]);

    next.flag = next.flag | (1 << 1);
    // write node
    bTree->write_node(next, arrayPosChild[index + 1]);

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
    Node prev = get_node(bTree, index - 1);
    Node next = get_node(bTree, index + 1);

    if (index != 0 && prev.used_keys >= t) {
        borrow_from_node_prev(bTree, index);
    } else if (index != used_keys && next.used_keys >= t) {
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
    Node prev = get_node(bTree, index - 1); // nho delete
    Node curr = get_node(bTree, index); // nho delete
    for (int i = curr.used_keys - 1; i >= 0; --i) {
        curr.arrayPosKey[i + 1] = curr.arrayPosKey[i];
    }
    if (!curr.is_leaf()) {
        for (int i = curr.used_keys; i >= 0; --i) {
            curr.arrayPosChild[i + 1] = curr.arrayPosChild[i];
        }
    }

    curr.arrayPosKey[0] = arrayPosKey[index - 1];
    if (!curr.is_leaf()) {

        curr.arrayPosChild[0] = prev.arrayPosChild[prev.used_keys];
    }
    arrayPosKey[index - 1] = prev.arrayPosKey[prev.used_keys - 1];
    curr.used_keys++;
    prev.used_keys--;

    bTree->write_node(curr, curr.m_pos);
    bTree->write_node(prev, prev.m_pos);
    bTree->write_node(*this, m_pos);
}

template<class K, class V>
void BTree<K,V>::BTreeNode::borrow_from_node_next(BTree* bTree, const int index) {
    Node curr = get_node(bTree, index);
    Node next = get_node(bTree, index + 1);

    curr.arrayPosKey[curr.used_keys] = arrayPosKey[index];

    if (!curr.is_leaf()) {
        curr.arrayPosChild[curr.used_keys + 1] = next.arrayPosChild[0];
    }

    arrayPosKey[index] = next.arrayPosKey[0];
    for (int i = 1; i < next.used_keys; ++i) {
        next.arrayPosKey[i - 1] = next.arrayPosKey[i];
    }
    if (!next.is_leaf()) {
        for (int i = 1; i <= next.used_keys; ++i) {
            next.arrayPosChild[i - 1] = next.arrayPosChild[i];
        }
    }
    curr.used_keys++;
    next.used_keys--;

    bTree->write_node(curr, curr.m_pos);
    bTree->write_node(next, next.m_pos);
    bTree->write_node(*this, m_pos);
}


