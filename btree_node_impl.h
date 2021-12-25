#pragma once

template<class K, class V>
BTree<K,V>::BTreeNode::BTreeNode(const int& t, bool isLeaf) :
    used_keys(0),
    t(t),
    flag(isLeaf ? 1 : 0),
    key_pos(max_key_num(), -1),
    child_pos(max_child_num(), -1)
{}

template<class K, class V>
bool BTree<K,V>::BTreeNode::is_leaf() const {
    return (flag & 1) == 1;
}

template<class K, class V>
bool BTree<K,V>::BTreeNode::is_full() const {
    return used_keys == max_key_num();
}

template <typename T>
inline void shift_right_by_one(std::vector<T>& v, const int from, const int to) {
//    assert(from >= 0 && to <= v.size() && from >= to);
    for (auto i = from; i > to; --i) {
        v[i] = v[i - 1];
    }
}

template <typename T>
inline void shift_left_by_one(std::vector<T>& v, const int from, const int to) {
//    assert(to >= 0 && from <= v.size() && from <= to);
    for (auto i = from; i < to; ++i) {
        v[i - 1] = v[i];
    }
}

template<class K, class V>
void BTree<K,V>::BTreeNode::split_child(IOManagerT& manager, const int idx, Node& curr_node) {
    // Create a new node to store (t-1) keys of divided node
    Node new_node(curr_node.t, curr_node.is_leaf());
    new_node.used_keys = t - 1;

    // Copy the last (t-1) keys of divided node to new_node
    for (auto i = 0; i < t - 1; ++i) {
        new_node.key_pos[i] = curr_node.key_pos[i + t];
        curr_node.key_pos[i + t] = -1;
    }
    // Copy the last (t-1) children of divided node to new_node
    if (!curr_node.is_leaf()) {
        for (auto i = 0; i < t; ++i) {
            new_node.child_pos[i] = curr_node.child_pos[i + t];
            curr_node.child_pos[i + t] = -1;
        }
    }

    new_node.m_pos = manager.get_file_pos_end();
    // write new node
    manager.write_node(new_node, new_node.m_pos);

    // Reduce the number of keys in the divided node
    curr_node.used_keys = t - 1;

    // write current node
    manager.write_node(curr_node, curr_node.m_pos);

    // Shift children, keys and values to right
    shift_right_by_one(child_pos, used_keys + 1, idx + 1);
    shift_right_by_one(key_pos, used_keys, idx);

    // set the key-divider
    key_pos[idx] = curr_node.key_pos[t - 1];
    child_pos[idx + 1] = new_node.m_pos;
    ++used_keys;

    // write node
    manager.write_node(*this, m_pos);
}

template<class K, class V>
K BTree<K,V>::BTreeNode::read_key(IOManagerT& io, const int idx) {
    if (idx < 0 || idx > used_keys - 1) {
        throw std::logic_error("Must have a key on idx: " + std::to_string(idx));
    }

    return io.read_key(key_pos[idx]);
}

template<class K, class V>
Entry<K, V> BTree<K,V>::BTreeNode::read_entry(IOManagerT& io, const int idx) {
    if (idx < 0 || idx > used_keys - 1) {
        throw std::logic_error("Must have an entry on idx: " + std::to_string(idx));
    }

    return io.read_entry(key_pos[idx]);
}

template<class K, class V>
typename BTree<K,V>::BTreeNode BTree<K,V>::BTreeNode::read_node(IOManagerT& io, const int idx) {
    if (idx < 0 || idx > used_keys) {
        return BTreeNode(0, false);
    }

    Node node(t, false);
    io.read_node(&node, child_pos[idx]);
    return node;
}

template<class K, class V>
void BTree<K,V>::BTreeNode::insert_non_full(IOManagerT& io, const EntryT& entry) {
    if (is_leaf()) {
        int idx = used_keys - 1;
        K curr_key = read_key(io, idx);

        while (idx >= 0 && curr_key > entry.key) {
            key_pos[idx + 1] = key_pos[idx];
            idx--;
            curr_key = read_key(io, idx);
        }
        int pos = io.get_file_pos_end(); // go to the end after reading entries
        key_pos[idx + 1] = pos;
        used_keys = used_keys + 1;

        //write node and entry
        io.write_entry(entry, pos);
        io.write_node(*this, m_pos);
    } else {
        int idx = find_key_bin_search(io, entry.key);
        Node node = read_node(io, idx);

        if (node.is_full()) {
            split_child(io, idx, node);
            K curr_key = read_key(io, idx);
            if (curr_key < entry.key) {
                idx++;
            }
        }

        node = read_node(io, idx);
        node.insert_non_full(io, entry);
    }
}

template<class K, class V>
int BTree<K,V>::BTreeNode::find_key_bin_search(IOManagerT& io, const K& key) {
    int left = 0;
    int right = used_keys - 1;
    int mid = 0;
    K tmp_key;

    while (left <= right) {
        mid = left + (right - left) / 2;
        tmp_key = read_key(io, mid);

        if (tmp_key < key) {
            left = mid + 1;
        } else if (tmp_key > key) {
            right = mid - 1;
        } else {
            return mid;
        }
    }
    return right + 1;
}

template<class K, class V>
Entry<K, V> BTree<K,V>::BTreeNode::find(IOManagerT& io, const K& key) {
    int idx = find_key_bin_search(io, key);

    if (idx < used_keys) {
        auto entry = read_entry(io, idx);
        if (entry.key == key) {
            return entry;
        }
    }

    if (is_leaf()) {
        return { EntryT::INVALID_KEY, EntryT::INVALID_VALUE };
    }

    Node child = read_node(io, idx);
    return child.find(io, key);
}

template<class K, class V>
bool BTree<K,V>::BTreeNode::set(IOManagerT& io, const K& key, const V& value) {
    int idx = find_key_bin_search(io, key);

    if (idx < used_keys) {
        K tmp_key = read_key(io, idx);
        if (tmp_key == key) {
            int curr_pos = io.get_file_pos_end();
            key_pos[idx] = curr_pos;

            io.write_entry({key, value}, curr_pos);
            io.write_node(*this, m_pos);

            return true;
        }
    }
    if (is_leaf()) {
        return false;
    }

    Node child = read_node(io, idx);
    return child.set(io, key, value);
}

template<class K, class V>
bool BTree<K,V>::BTreeNode::remove(IOManagerT& io, const K& key) {
    int idx = find_key_bin_search(io, key);
    K curr_key = read_key(io, idx);

    bool success;
    if (idx < used_keys && curr_key == key) {
        success = is_leaf() ? remove_from_leaf(io, idx) : remove_from_non_leaf(io, idx);
        io.write_node(*this, m_pos);
        return success;
    } else {
        if (is_leaf()) {
            return false;
        }
        // If the child where the key is supposed to exist has less that t keys, we fill that child
        Node node = read_node(io, idx);
        if (node.used_keys < t) {
            fill_node(io, idx);
            node = read_node(io, idx);
        }

        // If the last child has been merged, it must have merged with the previous child and
        //      so we recurse on the (pos-1)-th child.
        // Else, we recurse on the (pos)-th child which now has at least t keys
        int child_idx = (idx > used_keys) ? (idx - 1) : idx;
        node = read_node(io, child_idx);
        success = node.remove(io, key);
        io.write_node(node, node.m_pos);
    }
    return success;
}

template<class K, class V>
bool BTree<K,V>::BTreeNode::remove_from_leaf(IOManagerT& io, const int idx) {
    io.write_flag('0', key_pos[idx]);

    shift_left_by_one(key_pos, idx + 1, used_keys);
    --used_keys;
    return true;
}

template<class K, class V>
bool BTree<K,V>::BTreeNode::remove_from_non_leaf(IOManagerT& io, const int idx) {
    auto onExit = [&io](Node& curr, const int entry_pos) -> bool {
        K key = io.read_key(entry_pos);
        io.write_node(curr, curr.m_pos);
        return curr.remove(io, key);
    };

    // 1. If the child[pos] has >= T keys, find the PREVIOUS in the subtree rooted at child[pos].
    // 2. Replace keys[pos], values[pos] by the PREVIOUS[key|value].
    // 3. Recursively delete PREVIOUS in child[pos].
    if (Node curr = read_node(io, idx); curr.used_keys >= t) {
        int entry_pos = get_prev_entry_pos(io, idx);
        key_pos[idx] = entry_pos;
        return onExit(curr, entry_pos);
    // If the child[pos] has <= T keys, check the child[pos + 1].
    // 1. If child[pos + 1] has >= T keys, find the NEXT in the subtree rooted at child[pos + 1].
    // 2. Replace keys[pos], values[pos] by the NEXT[key|value].
    // 3. Recursively delete NEXT in child[pos + 1].
    } else if (Node next = read_node(io, idx + 1);  next.used_keys >= t) {
        int entry_pos = get_next_entry_pos(io, idx);
        key_pos[idx] = entry_pos;
        return onExit(next, entry_pos);
    // 1. Now child[pos] and child[pos + 1] has < T keys.
    // 2. Merge key and child[pos + 1] into child[pos].
    // 3. Now child[pos] has (2 * t - 1) keys
    // 4. Recursively delete KEY from child[pos]
    } else {
        merge_node(io, idx);
        Node node = read_node(io, idx);
        return onExit(node, idx);
    }
}

template<class K, class V>
int BTree<K,V>::BTreeNode::get_prev_entry_pos(IOManagerT& io, const int index) {
    Node curr(t, false);
    io.read_node(&curr, child_pos[index]);
    // Keep moving to the right most node until CURR becomes a leaf
    while (!curr.is_leaf()) {
        io.read_node(&curr, curr.child_pos[curr.used_keys]);
    }

    return curr.key_pos[curr.used_keys - 1];
}

template<class K, class V>
int BTree<K,V>::BTreeNode::get_next_entry_pos(IOManagerT& io, const int index) {
    Node curr(t, false);
    io.read_node(&curr, child_pos[index + 1]);
    // Keep moving the left most node until CURR becomes a leaf
    while (!curr.is_leaf()) {
        io.read_node(&curr, curr.child_pos[0]);
    }

    return curr.key_pos[0];
}

template<class K, class V>
void BTree<K,V>::BTreeNode::merge_node(IOManagerT& io, const int idx) {
    Node curr = read_node(io, idx);
    Node next = read_node(io, idx + 1);

    curr.key_pos[t - 1] = key_pos[idx];

    // Copy all from child[pos + 1] to child[pos]
    for (auto i = 0; i < next.used_keys; ++i) {
        curr.key_pos[i + t] = next.key_pos[i];
    }

    // Copy the children from child[pos + 1] to child[pos]
    if (!curr.is_leaf()) {
        for (auto i = 0; i <= next.used_keys; ++i) {
            curr.child_pos[i + t] = next.child_pos[i];
        }
    }

    curr.used_keys = curr.used_keys + next.used_keys + 1;

    // write node
    io.write_node(curr, child_pos[idx]);

    next.flag = next.flag | (1 << 1);
    // write node
    io.write_node(next, child_pos[idx + 1]);

    shift_left_by_one(key_pos, idx + 1, used_keys);
    shift_left_by_one(child_pos, idx + 2, used_keys + 1);

    used_keys--;
    io.write_node(*this, m_pos);
}

template<class K, class V>
void BTree<K,V>::BTreeNode::fill_node(IOManagerT& io, const int index) {
    Node prev = read_node(io, index - 1);
    Node next = read_node(io, index + 1);

    // If the left child has >= (T - 1) keys, borrow a key from it
    if (index != 0 && prev.used_keys >= t) {
        borrow_from_prev_node(io, index);

    // If the right child has >= (T - 1) keys, borrow a key from it
    } else if (index != used_keys && next.used_keys >= t) {
        borrow_from_next_node(io, index);

    // Merge child[pos] with its sibling
    // If child[pos] is the last child, merge it with its previous sibling
    // Otherwise merge it with its next sibling
    } else {
        int idx = (index != used_keys) ? index : index - 1;
        merge_node(io, idx);
    }
}

template<class K, class V>
void BTree<K,V>::BTreeNode::borrow_from_prev_node(IOManagerT& io, const int idx) {
    Node prev = read_node(io, idx - 1);
    Node curr = read_node(io, idx);

    shift_right_by_one(curr.key_pos, curr.used_keys, 0);

    if (!curr.is_leaf()) {
        shift_right_by_one(curr.child_pos, curr.used_keys + 1, 0);
    }
    curr.key_pos[0] = key_pos[idx - 1];

    if (!curr.is_leaf()) {
        curr.child_pos[0] = prev.child_pos[prev.used_keys];
    }
    key_pos[idx - 1] = prev.key_pos[prev.used_keys - 1];

    curr.used_keys++;
    prev.used_keys--;

    io.write_node(prev, prev.m_pos);
    io.write_node(curr, curr.m_pos);
    io.write_node(*this, m_pos);
}

template<class K, class V>
void BTree<K,V>::BTreeNode::borrow_from_next_node(IOManagerT& io, const int idx) {
    Node curr = read_node(io, idx);
    Node next = read_node(io, idx + 1);
    curr.key_pos[curr.used_keys] = key_pos[idx];

    if (!curr.is_leaf()) {
        curr.child_pos[curr.used_keys + 1] = next.child_pos[0];
    }

    key_pos[idx] = next.key_pos[0];
    shift_left_by_one(next.key_pos, 1, next.used_keys);
    if (!next.is_leaf()) {
        shift_left_by_one(next.child_pos, 1, next.used_keys + 1);
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

    for (i = 0; i < used_keys; ++i) {
        if (!is_leaf()) {
            node = read_node(io, i);
            cout << endl;
            node.traverse(io);
            cout << endl;
        }
        auto entry = read_entry(io, i);
        cout << "[key]: " << entry->key << " - [value]: " << entry->value << " ";
    }
    if (!is_leaf()) {
        node = read_node(io, i);
        cout << endl;
        node.traverse(io);
        cout << endl;
    }
}
