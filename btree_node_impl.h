#pragma once

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
void BTree<K,V>::BTreeNode::split_child(IOManagerT& manager, const int idx, Node& curr_node) {
    // Create a new node to store (t-1) keys of divided node
    Node new_node(curr_node.t, curr_node.is_leaf());
    new_node.used_keys = t - 1;

    // Copy the last (t-1) keys of divided node to new_node
    for (int i = 0; i < t - 1; ++i) {
        new_node.arrayPosKey[i] = curr_node.arrayPosKey[i + t];
    }
    // Copy the last (t-1) children of divided node to new_node
    if (!curr_node.is_leaf()) {
        for (int i = 0; i < t; ++i) {
            new_node.arrayPosChild[i] = curr_node.arrayPosChild[i + t];
        }
    }

    const int pos = manager.get_file_pos_end(); // + setPosEnd
    new_node.m_pos = pos;
    // write new node
    manager.write_node(new_node, new_node.m_pos);

    // Reduce the number of keys in the divided node

    curr_node.used_keys = t - 1;

    // write current node
    manager.write_node(curr_node, curr_node.m_pos);

    for (int i = used_keys; i >= idx + 1; --i) {
        arrayPosChild[i + 1] = arrayPosChild[i];
    }
    arrayPosChild[idx + 1] = new_node.m_pos;

    for (int i = used_keys - 1; i >= idx; --i) {
        arrayPosKey[i + 1] = arrayPosKey[i];
    }
    // set the key-divider
    arrayPosKey[idx] = curr_node.arrayPosKey[t - 1];
    ++used_keys;

    // write node
    manager.write_node(*this, m_pos);
}

template<class K, class V>
Entry<K, V> BTree<K,V>::BTreeNode::read_entry(IOManagerT& io, const int i) {
    if (i < 0 || i > used_keys - 1) {
        return {}; // nullptr
    }

    int pos = arrayPosKey[i];
    EntryT entry;
    io.read_entry(entry, pos);
    return entry;
}

template<class K, class V>
typename BTree<K,V>::BTreeNode BTree<K,V>::BTreeNode::get_node(IOManagerT& io, const int i) {
    if (i < 0 || i > used_keys) {
        return Node(1, false); // dummy
    }

    int pos = arrayPosChild[i];
    Node node(t, false);

    //read node
    io.read_node(&node, pos);
    return node;
}

template<class K, class V>
void BTree<K,V>::BTreeNode::insert_non_full(IOManagerT& io, const EntryT& entry) {

    if (is_leaf()) {
        int idx = used_keys - 1;
        EntryT entryTmp = read_entry(io, idx);

        while (idx >= 0 && entryTmp.key > entry.key) {
            arrayPosKey[idx + 1] = arrayPosKey[idx];
            idx--;
            entryTmp = read_entry(io, idx);
        }

        int pos = io.get_file_pos_end(); // + setPosEnd

        //write entry
        io.write_entry(entry, pos);

        arrayPosKey[idx + 1] = pos;
        ++used_keys;

        //write node
        io.write_node(*this, m_pos);
    } else {
        int idx = find_key_bin_search(io, entry.key);

        Node node = get_node(io, idx);

        if (node.is_full()) {
            split_child(io, idx, node);

            Entry entryTmp = read_entry(io, idx);

            if (entryTmp.key < entry.key) {
                idx++;
            }
        }

        node = get_node(io, idx);
        node.insert_non_full(io, entry);
    }
}

template<class K, class V>
int BTree<K,V>::BTreeNode::find_key_bin_search(IOManagerT& io, const K& key) {
    int low = 0;
    int hight = used_keys - 1;
    int middle = (low + hight) / 2;
    EntryT entry = read_entry(io, middle);

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
        entry = read_entry(io, middle);
    }
    return hight + 1;
}


template<class K, class V>
Entry<K, V> BTree<K,V>::BTreeNode::find(IOManagerT& io, const K& key) {
    int i = 0;
    EntryT entry = read_entry(io, i);
    while (i < used_keys && entry.key < key) {
        ++i;
        entry = read_entry(io, i);
    }

    if (i < used_keys && entry.key == key) {
        return entry;
    }

    if (is_leaf()) {
        return {}; // nullptr dummy
    }

    Node node = get_node(io, i);
    return node.find(io, key);
}

template<class K, class V>
bool BTree<K,V>::BTreeNode::set(IOManagerT& io, const K& key, const V& value) {
    int idx = find_key_bin_search(io, key);

    if (idx < used_keys) {
        EntryT entry = read_entry(io, idx);
        if (entry.key == key) {
            if (entry.value != value) {
//                int node_start_pos = arrayPosKey[idx] - io.get_node_size_in_bytes(*this);
                io.write_flag('0', arrayPosKey[idx]);

                int curr_pos = io.get_file_pos_end();
                arrayPosKey[idx] = curr_pos;
                entry.value = value;

                io.write_entry(entry, curr_pos);
                io.write_node(*this, m_pos);
            }

            return true;
        }
    }
    if (is_leaf()) {
        return false;
    }

    Node node = get_node(io, idx);

    return node.set(io, key, value);
}

template<class K, class V>
bool BTree<K,V>::BTreeNode::remove(IOManagerT& io, const K& key) {
    int idx = find_key_bin_search(io, key);
    EntryT entry = read_entry(io, idx);

    bool success;
    if (idx < used_keys && entry.key == key) {
        success = is_leaf() ? remove_from_leaf(io, idx) : remove_from_non_leaf(io, idx);
        io.write_node(*this, m_pos);
    } else {
        if (is_leaf()) {
            return false;
        }

        auto child = get_node(io, idx);

        if (child.used_keys < t) {
            fill_node(io, idx);
        }

        int child_idx = (idx > used_keys) ? (idx - 1) : idx;
        auto m_child  = get_node(io, child_idx);

        if (m_child.t != 1) {
            success =  m_child.remove(io, key);
            io.write_node(m_child, m_child.m_pos);
        }
    }
    return success;
}

template<class K, class V>
bool BTree<K,V>::BTreeNode::remove_from_leaf(IOManagerT& io, const int idx) {
    io.write_flag('0', arrayPosKey[idx]);

    // shift to the left by 1 all the keys after the pos
    for (int i = idx + 1; i < used_keys; ++i) {
        arrayPosKey[i - 1] = arrayPosKey[i];
    }
    --used_keys;
    return true;
}

template<class K, class V>
bool BTree<K,V>::BTreeNode::remove_from_non_leaf(IOManagerT& io, const int index) {
    Node curr = get_node(io, index);
    Node next = get_node(io, index + 1);

    int curr_pos;
    bool res;

    // 1. If the child[pos] has >= T keys, find the PREVIOUS in the subtree rooted at child[pos].
    // 2. Replace keys[pos], values[pos] by the PREVIOUS[key|value].
    // 3. Recursively delete PREVIOUS in child[pos].
    if (curr.used_keys >= t) {
        curr_pos = get_prev_entry_pos(io, index);
        arrayPosKey[index] = curr_pos;

        EntryT entry;
        io.read_entry(entry, curr_pos);
        K key = entry.key;

        res = curr.remove(io, key);
    // If the child[pos] has <= T keys, check the child[pos + 1].
    // 1. If child[pos + 1] has >= T keys, find the NEXT in the subtree rooted at child[pos + 1].
    // 2. Replace keys[pos], values[pos] by the NEXT[key|value].
    // 3. Recursively delete NEXT in child[pos + 1].
    } else if (next.used_keys >= t) {
        curr_pos = get_next_entry_pos(io, index);
        arrayPosKey[index] = curr_pos;

        EntryT entry;
        io.read_entry(entry, curr_pos);
        K key = entry.key;

        res = next.remove(io, key);
    // 1. Now child[pos] and child[pos + 1] has < T keys.
    // 2. Merge key and child[pos + 1] into child[pos].
    // 3. Now child[pos] has (2 * t - 1) keys
    // 4. Recursively delete KEY from child[pos]
    } else {
        EntryT entry = read_entry(io, index);
        merge_node(io, index);
        K key = entry.key;

        curr = get_node(io, index);
        res = curr.remove(io, key);
    }

    io.write_node(curr, curr.m_pos);

    return res;
}

template<class K, class V>
int BTree<K,V>::BTreeNode::get_prev_entry_pos(IOManagerT& io, const int index) {
    Node curr(t, false);
    io.read_node(&curr, arrayPosChild[index]);
    // Keep moving to the right most node until CURR becomes a leaf
    while (!curr.is_leaf()) {
        io.read_node(&curr, curr.arrayPosChild[curr.used_keys]);
    }

    return curr.arrayPosKey[curr.used_keys - 1];
}

template<class K, class V>
int BTree<K,V>::BTreeNode::get_next_entry_pos(IOManagerT& io, const int index) {
    Node curr(t, false);
    io.read_node(&curr, arrayPosChild[index + 1]);
    // Keep moving the left most node until CURR becomes a leaf
    while (!curr.is_leaf()) {
        io.read_node(&curr, curr.arrayPosChild[0]);
    }

    return curr.arrayPosKey[0];
}

template<class K, class V>
void BTree<K,V>::BTreeNode::merge_node(IOManagerT& io, const int idx) {
    Node curr = get_node(io, idx);
    Node next = get_node(io, idx + 1);

    curr.arrayPosKey[t - 1] = arrayPosKey[idx];

    // Copy all from child[pos + 1] to child[pos]
    for (int i = 0; i < next.used_keys; ++i) {
        curr.arrayPosKey[i + t] = next.arrayPosKey[i];
    }

    // Copy the children from child[pos + 1] to child[pos]
    if (!curr.is_leaf()) {
        for (int i = 0; i <= next.used_keys; ++i) {
            curr.arrayPosChild[i + t] = next.arrayPosChild[i];
        }
    }

    curr.used_keys += next.used_keys + 1;

    // write node
    io.write_node(curr, arrayPosChild[idx]);

    next.flag = next.flag | (1 << 1);
    // write node
    io.write_node(next, arrayPosChild[idx + 1]);

    for (int i = idx + 1; i < used_keys; ++i) {
        arrayPosKey[i - 1] = arrayPosKey[i];
    }
    for (int i = idx + 1; i < used_keys; ++i) {
        arrayPosChild[i] = arrayPosChild[i + 1];
    }

    used_keys--;
    io.write_node(*this, m_pos);
}

template<class K, class V>
void BTree<K,V>::BTreeNode::fill_node(IOManagerT& io, const int index) {
    Node prev = get_node(io, index - 1);
    Node next = get_node(io, index + 1);

    // If the left child has >= (T - 1) keys, borrow a key from it
    if (index != 0 && prev.used_keys >= t) {
        borrow_from_node_prev(io, index);

    // If the right child has >= (T - 1) keys, borrow a key from it
    } else if (index != used_keys && next.used_keys >= t) {
        borrow_from_node_next(io, index);

    // Merge child[pos] with its sibling
    // If child[pos] is the last child, merge it with its previous sibling
    // Otherwise merge it with its next sibling
    } else {
        int idx = (index != used_keys) ? index : index - 1;
        merge_node(io, idx);
    }
}

template<class K, class V>
void BTree<K,V>::BTreeNode::borrow_from_node_prev(IOManagerT& io, const int index) {
    Node prev = get_node(io, index - 1);
    Node curr = get_node(io, index);
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

    io.write_node(curr, curr.m_pos);
    io.write_node(prev, prev.m_pos);
    io.write_node(*this, m_pos);
}

template<class K, class V>
void BTree<K,V>::BTreeNode::borrow_from_node_next(IOManagerT& io, const int index) {
    Node curr = get_node(io, index);
    Node next = get_node(io, index + 1);
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

    io.write_node(curr, curr.m_pos);
    io.write_node(next, next.m_pos);
    io.write_node(*this, m_pos);
}

template<class K, class V>
void BTree<K,V>::BTreeNode::traverse(IOManagerT& io) {
    int i;
    Node node;
    EntryT entry;
    for (i = 0; i < used_keys; ++i) {
        if (!is_leaf()) {
            node = get_node(io, i);
//            cout << endl;
            node.traverse(io);
//            cout << endl;
        }
        entry = read_entry(io, i);
//        cout << "[key]: " << entry->key << " - [value]: " << entry->value << " ";
    }
    if (!is_leaf()) {
        node = get_node(io, i);
//        cout << endl;
        node.traverse(io);
//        cout << endl;
    }
}
