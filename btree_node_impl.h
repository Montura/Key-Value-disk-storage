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
void BTree<K,V>::BTreeNode::split_child(IOManagerT& io_manager, const int index, Node& curr_node) {
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

    io_manager.setPosEndFile();
    const int pos = io_manager.getPosFile();
    new_node.m_pos = pos;
    // write new node
    io_manager.write_node(new_node, new_node.m_pos);

    curr_node.used_keys = t - 1;

    // write current node
    io_manager.write_node(curr_node, curr_node.m_pos);

    for (int i = used_keys; i >= index + 1; --i) {
        arrayPosChild[i + 1] = arrayPosChild[i];
    }
    arrayPosChild[index + 1] = new_node.m_pos;

    for (int i = used_keys - 1; i >= index; --i) {
        arrayPosKey[i + 1] = arrayPosKey[i];
    }

    arrayPosKey[index] = curr_node.arrayPosKey[t - 1];
    used_keys = used_keys + 1;

    // write node
    io_manager.write_node(*this, m_pos);
}

template<class K, class V>
Entry<K, V> BTree<K,V>::BTreeNode::read_entry(IOManagerT& io_manager, const int i) {
    if (i < 0 || i > used_keys - 1) {
        assert(false);
        throw std::logic_error("Failed condition i < 0 || i > used_keys - 1");
    }

    int pos = arrayPosKey[i];
    EntryT entry;
    io_manager.read_entry(entry, pos);
    return entry;
}

template<class K, class V>
typename BTree<K,V>::BTreeNode BTree<K,V>::BTreeNode::get_node(IOManagerT& io_manager, const int i) {
    if (i < 0 || i > used_keys) {
        return BTreeNode(1, false);
    }

    int pos = arrayPosChild[i];
    Node node(t, false);

    //read node
    io_manager.read_node(&node, pos);
    return node;
}

template<class K, class V>
void BTree<K,V>::BTreeNode::insert_non_full(IOManagerT& io_manager, const EntryT& entry) {

    if (is_leaf()) {
        int idx = used_keys - 1;
        EntryT entryTmp = read_entry(io_manager, idx);

        while (idx >= 0 && entryTmp.key > entry.key) {
            arrayPosKey[idx + 1] = arrayPosKey[idx];
            idx--;
            entryTmp = read_entry(io_manager, idx);
        }

        io_manager.setPosEndFile();
        int pos = io_manager.getPosFile();

        //write entry
        io_manager.write_entry(entry, pos);

        arrayPosKey[idx + 1] = pos;
        used_keys = used_keys + 1;

        //write node
        io_manager.write_node(*this, m_pos);
    } else {
        int idx = find_key_bin_search(io_manager, entry.key);

        Node node = get_node(io_manager, idx);

        if (node.is_full()) {
            split_child(io_manager, idx, node);

            Entry entryTmp = read_entry(io_manager, idx);

            if (entryTmp.key < entry.key) {
                idx++;
            }
        }

        node = get_node(io_manager, idx);
        node.insert_non_full(io_manager, entry);
    }
}

template<class K, class V>
void BTree<K,V>::BTreeNode::traverse(IOManagerT& io_manager) {
    int i;
    Node node;
    EntryT entry;
    for (i = 0; i < used_keys; ++i) {
        if (!is_leaf()) {
            node = get_node(io_manager, i);
            cout << endl;
            node.traverse(io_manager);
            cout << endl;
        }
        entry = read_entry(io_manager, i);
        cout << "[key]: " << entry->key << " - [value]: " << entry->value << " ";
    }
    if (!is_leaf()) {
        node = get_node(io_manager, i);
        cout << endl;
        node.traverse(io_manager);
        cout << endl;
    }
}

template<class K, class V>
int BTree<K,V>::BTreeNode::find_key_bin_search(IOManagerT& io_manager, const K& key) {
    int low = 0;
    int hight = used_keys - 1;
    int middle = (low + hight) / 2;
    EntryT entry = read_entry(io_manager, middle);

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
        entry = read_entry(io_manager, middle);
    }
    return hight + 1;
}


template<class K, class V>
Entry<K, V> BTree<K,V>::BTreeNode::find(IOManagerT& io_manager, const K& key) {
    int idx = find_key_bin_search(io_manager, key);

    if (idx < used_keys) {
        auto entry = read_entry(io_manager, idx);
        if (entry.key == key) {
            return entry;
        }
    }

    if (is_leaf()) {
        return { EntryT::INVALID_KEY, EntryT::INVALID_VALUE };
    }

    Node node = get_node(io_manager, idx);
    return node.find(io_manager, key);
}

template<class K, class V>
bool BTree<K,V>::BTreeNode::set(IOManagerT& io_manager, const K& key, const V& value) {
    int idx = find_key_bin_search(io_manager, key);

    if (idx < used_keys) {
        EntryT entry = read_entry(io_manager, idx);
        if (entry.key == key) {
            io_manager.write_flag('0', arrayPosKey[idx]);

            io_manager.setPosEndFile();
            int curr_pos = io_manager.getPosFile();

            arrayPosKey[idx] = curr_pos;
            entry.value = value;

            io_manager.write_entry(entry, curr_pos);
            io_manager.write_node(*this, m_pos);

            return true;
        }
    }
    if (is_leaf()) {
        return false;
    }

    Node node = get_node(io_manager, idx);

    return node.set(io_manager, key, value);
}

template<class K, class V>
bool BTree<K,V>::BTreeNode::remove(IOManagerT& io_manager, const K& key) {
    int idx = find_key_bin_search(io_manager, key);
    EntryT entry = read_entry(io_manager, idx);

    bool success;
    if (idx < used_keys && entry.key == key) {
        success = is_leaf() ? remove_from_leaf(io_manager, idx) : remove_from_non_leaf(io_manager, idx);
        io_manager.write_node(*this, m_pos);
        return success;
    } else if (!is_leaf()) {
        Node node = get_node(io_manager, idx);

        if (node.used_keys < t) {
            fill_node(io_manager, idx);
            node = get_node(io_manager, idx);
        }

        success = node.remove(io_manager, key);
        io_manager.write_node(node, node.m_pos);
        return success;
    } else {
        return false;
    }
}

template<class K, class V>
bool BTree<K,V>::BTreeNode::remove_from_leaf(IOManagerT& io_manager, const int index) {
    io_manager.write_flag('0', arrayPosKey[index]);

    for (int i = index + 1; i < used_keys; ++i) {
        arrayPosKey[i - 1] = arrayPosKey[i];
    }
    used_keys--;

    return true;
}

template<class K, class V>
bool BTree<K,V>::BTreeNode::remove_from_non_leaf(IOManagerT& io_manager, const int index) {
    Node curr = get_node(io_manager, index);
    Node next = get_node(io_manager, index + 1);

    int curr_pos;
    bool res;

    if (curr.used_keys >= t) {
        curr_pos = get_prev_entry_pos(io_manager, index);
        arrayPosKey[index] = curr_pos;

        EntryT entry;
        io_manager.read_entry(entry, curr_pos);
        K key = entry.key;

        res = curr.remove(io_manager, key);
    } else if (next.used_keys >= t) {
        curr_pos = get_next_entry_pos(io_manager, index);
        arrayPosKey[index] = curr_pos;

        EntryT entry;
        io_manager.read_entry(entry, curr_pos);
        K key = entry.key;

        res = next.remove(io_manager, key);
    } else {
        EntryT entry = read_entry(io_manager, index);
        merge_node(io_manager, index);
        K key = entry.key;

        curr = get_node(io_manager, index);
        res = curr.remove(io_manager, key);
    }

    io_manager.write_node(curr, curr.m_pos);

    return res;
}

template<class K, class V>
int BTree<K,V>::BTreeNode::get_prev_entry_pos(IOManagerT& io_manager, const int index) {
    Node curr(t, false);
    io_manager.read_node(&curr, arrayPosChild[index]);
    while (!curr.is_leaf()) {
        io_manager.read_node(&curr, curr.arrayPosChild[curr.used_keys]);
    }

    return curr.arrayPosKey[curr.used_keys - 1];
}

template<class K, class V>
int BTree<K,V>::BTreeNode::get_next_entry_pos(IOManagerT& io_manager, const int index) {
    Node curr(t, false);
    io_manager.read_node(&curr, arrayPosChild[index + 1]);
    while (!curr.is_leaf()) {
        io_manager.read_node(&curr, curr.arrayPosChild[0]);
    }

    return curr.arrayPosKey[0];
}

template<class K, class V>
void BTree<K,V>::BTreeNode::merge_node(IOManagerT& io_manager, const int index) {
    Node curr = get_node(io_manager, index);
    Node next = get_node(io_manager, index + 1);

    curr.arrayPosKey[t - 1] = arrayPosKey[index];

    for (int i = 0; i < next.used_keys; ++i) {
        curr.arrayPosKey[i + t] = next.arrayPosKey[i];
    }
    if (!curr.is_leaf()) {
        for (int i = 0; i <= next.used_keys; ++i) {
            curr.arrayPosChild[i + t] = next.arrayPosChild[i];
        }
    }

    curr.used_keys = curr.used_keys + next.used_keys + 1;

    // write node
    io_manager.write_node(curr, arrayPosChild[index]);

    next.flag = next.flag | (1 << 1);
    // write node
    io_manager.write_node(next, arrayPosChild[index + 1]);

    for (int i = index + 1; i < used_keys; ++i) {
        arrayPosKey[i - 1] = arrayPosKey[i];
    }
    for (int i = index + 1; i < used_keys; ++i) {
        arrayPosChild[i] = arrayPosChild[i + 1];
    }

    used_keys--;
    io_manager.write_node(*this, m_pos);
}

template<class K, class V>
void BTree<K,V>::BTreeNode::fill_node(IOManagerT& io_manager, const int index) {
    Node prev = get_node(io_manager, index - 1);
    Node next = get_node(io_manager, index + 1);

    if (index != 0 && prev.used_keys >= t) {
        borrow_from_node_prev(io_manager, index);
    } else if (index != used_keys && next.used_keys >= t) {
        borrow_from_node_next(io_manager, index);
    } else {
        if (index != used_keys) {
            merge_node(io_manager, index);
        } else {
            merge_node(io_manager, index - 1);
        }
    }

}

template<class K, class V>
void BTree<K,V>::BTreeNode::borrow_from_node_prev(IOManagerT& io_manager, const int index) {
    Node prev = get_node(io_manager, index - 1); // nho delete
    Node curr = get_node(io_manager, index); // nho delete
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

    io_manager.write_node(curr, curr.m_pos);
    io_manager.write_node(prev, prev.m_pos);
    io_manager.write_node(*this, m_pos);
}

template<class K, class V>
void BTree<K,V>::BTreeNode::borrow_from_node_next(IOManagerT& io_manager, const int index) {
    Node curr = get_node(io_manager, index);
    Node next = get_node(io_manager, index + 1);
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

    io_manager.write_node(curr, curr.m_pos);
    io_manager.write_node(next, next.m_pos);
    io_manager.write_node(*this, m_pos);
}


