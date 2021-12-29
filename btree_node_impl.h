#pragma once

template<class K, class V>
BTree<K,V>::BTreeNode::BTreeNode(const int& t, bool isLeaf) :
    used_keys(0),
    t(t),
    m_pos(-1),
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
Entry<K, V> BTree<K,V>::BTreeNode::get_entry(IOManagerT& io, const int idx) {
    if (idx < 0 || idx > used_keys - 1)
        return {}; // nullptr

    return io.read_entry(key_pos[idx]);
}

template<class K, class V>
typename BTree<K,V>::BTreeNode BTree<K,V>::BTreeNode::get_child(IOManagerT& io, const int idx) {
    if (idx < 0 || idx > used_keys)
        return Node(0, false); // dummy

    return io.read_node(child_pos[idx]);
}

template<class K, class V>
void BTree<K,V>::BTreeNode::insert_non_full(IOManagerT& io, const K& key, const V& value) {
    if (is_leaf()) {
        int idx = used_keys - 1;
        K curr_key = get_entry(io, idx).key;

        while (idx >= 0 && curr_key > key) {
            key_pos[idx + 1] = key_pos[idx];
            idx--;
            curr_key = get_entry(io, idx).key;
        }

        int pos = io.get_file_pos_end(); // go to the end after reading entries
        key_pos[idx + 1] = pos;
        ++used_keys;

        // Write node and entry
        io.write_entry({key, value}, pos);
        io.write_node(*this, m_pos);
    } else {
        int idx = find_key_bin_search(io, key);
        Node node = get_child(io, idx);

        if (node.is_full()) {
            split_child(io, idx, node);
            K curr_key = get_entry(io, idx).key;
            if (curr_key < key)
                idx++;
        }

        node = get_child(io, idx);
        node.insert_non_full(io, key, value);
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
        tmp_key = get_entry(io, mid).key;

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
    Node curr = *this;

    while (!curr.is_leaf()) {
        auto idx = curr.find_key_bin_search(io, key);

        if (idx < curr.used_keys) {
            auto entry = curr.get_entry(io, idx);
            if (entry.key == key)
                return entry;
        }
        curr = curr.get_child(io, idx);
    }

    auto idx = curr.find_key_bin_search(io, key);

    if (idx < curr.used_keys) {
        auto entry = curr.get_entry(io, idx);
        if (entry.key == key)
            return entry;
    }

    return {}; // nullptr dummy
}

template<class K, class V>
bool BTree<K,V>::BTreeNode::set(IOManagerT& io, const K& key, const V& value) {
    int idx = find_key_bin_search(io, key);

    if (idx < used_keys) {
        EntryT entry = get_entry(io, idx);
        if (entry.key == key) {
            if (entry.value != value) {
                io.write_flag('0', key_pos[idx]);

                int curr_pos = io.get_file_pos_end();
                key_pos[idx] = curr_pos;
                entry.value = value;

                io.write_entry(std::move(entry), curr_pos);
                io.write_node(*this, m_pos);
            }

            return true;
        }
    }
    if (is_leaf())
        return false;

    Node child = get_child(io, idx);
    return child.set(io, key, value);
}

template<class K, class V>
bool BTree<K,V>::BTreeNode::remove(IOManagerT& io, const K& key) {
    int idx = find_key_bin_search(io, key);
    K curr_key = get_entry(io, idx).key;

    bool success;
    if (idx < used_keys && curr_key == key) {
        success = is_leaf() ? remove_from_leaf(io, idx) : remove_from_non_leaf(io, idx);
        io.write_node(*this, m_pos);
    } else {
        if (is_leaf())
            return false;

        // If the child where the key is supposed to exist has less that t keys, we fill that child
        auto child = get_child(io, idx);
        if (child.used_keys < t)
            fill_node(io, idx);

        int child_idx = (idx > used_keys) ? (idx - 1) : idx;
        auto m_child  = get_child(io, child_idx);

        if (m_child.t != 0) {
            success =  m_child.remove(io, key);
            io.write_node(m_child, m_child.m_pos);
        }
    }
    return success;
}

template<class K, class V>
bool BTree<K,V>::BTreeNode::remove_from_leaf(IOManagerT& io, const int idx) {
    io.write_flag('0', key_pos[idx]);

    // shift to the left by 1 all the keys after the pos
    shift_left_by_one(key_pos, idx + 1, used_keys);
    --used_keys;
    return true;
}

template<class K, class V>
bool BTree<K,V>::BTreeNode::remove_from_non_leaf(IOManagerT& io, const int idx) {
    Node curr = get_child(io, idx);
    Node next = get_child(io, idx + 1);

    int curr_pos;
    bool res;

    // 1. If the child[pos] has >= T keys, find the PREVIOUS in the subtree rooted at child[pos].
    // 2. Replace keys[pos], values[pos] by the PREVIOUS[key|value].
    // 3. Recursively delete PREVIOUS in child[pos].
    if (curr.used_keys >= t) {
        curr_pos = get_prev_entry_pos(io, idx);
        key_pos[idx] = curr_pos;
        K key = io.read_entry(curr_pos).key;
        res = curr.remove(io, key);
    // If the child[pos] has <= T keys, check the child[pos + 1].
    // 1. If child[pos + 1] has >= T keys, find the NEXT in the subtree rooted at child[pos + 1].
    // 2. Replace keys[pos], values[pos] by the NEXT[key|value].
    // 3. Recursively delete NEXT in child[pos + 1].
    } else if (next.used_keys >= t) {
        curr_pos = get_next_entry_pos(io, idx);
        key_pos[idx] = curr_pos;
        K key = io.read_entry(curr_pos).key;
        res = next.remove(io, key);
    // 1. Now child[pos] and child[pos + 1] has < T keys.
    // 2. Merge key and child[pos + 1] into child[pos].
    // 3. Now child[pos] has (2 * t - 1) keys
    // 4. Recursively delete KEY from child[pos]
    } else {
        K key = get_entry(io, idx).key;
        merge_node(io, idx);

        curr = get_child(io, idx);
        res = curr.remove(io, key);
    }

    io.write_node(curr, curr.m_pos);

    return res;
}

template<class K, class V>
int BTree<K,V>::BTreeNode::get_prev_entry_pos(IOManagerT& io, const int idx) {
    Node curr = io.read_node(child_pos[idx]);
    // Keep moving to the right most node until CURR becomes a leaf
    while (!curr.is_leaf())
        curr = io.read_node(curr.child_pos[curr.used_keys]);

    return curr.key_pos[curr.used_keys - 1];
}

template<class K, class V>
int BTree<K,V>::BTreeNode::get_next_entry_pos(IOManagerT& io, const int idx) {
    Node curr = io.read_node(child_pos[idx + 1]);
    // Keep moving the left most node until CURR becomes a leaf
    while (!curr.is_leaf())
        curr = io.read_node(curr.child_pos[0]);

    return curr.key_pos[0];
}

template<class K, class V>
char BTree<K,V>::BTreeNode::is_deleted_or_is_leaf() const {
    return flag | (1 << 1);
}

template<class K, class V>
void BTree<K,V>::BTreeNode::merge_node(IOManagerT& io, const int idx) {
    Node child = get_child(io, idx);
    Node next_child = get_child(io, idx + 1);

    // Set the key from CURR node to (t-1)th pos of child
    child.key_pos[t - 1] = key_pos[idx];

    // Copy all keys from NEXT to CHILD
    for (auto i = 0; i < next_child.used_keys; ++i)
        child.key_pos[i + t] = next_child.key_pos[i];

    // Copy all children from NEXT to CHILD
    if (!child.is_leaf()) {
        for (auto i = 0; i <= next_child.used_keys; ++i)
            child.child_pos[i + t] = next_child.child_pos[i];
    }

    // Increment CHILD's key count and write it
    child.used_keys += next_child.used_keys + 1;

    // write node
    io.write_node(child, child_pos[idx]);

    next_child.flag = next_child.flag | (1 << 1);
    // write node
    io.write_node(next_child, child_pos[idx + 1]);

    // Update KEYs and CHILDREN for CURR
    shift_left_by_one(key_pos, idx + 1, used_keys);
    shift_left_by_one(child_pos, idx + 2, used_keys + 1);
    used_keys--;
    io.write_node(*this, m_pos);
}

template<class K, class V>
void BTree<K,V>::BTreeNode::fill_node(IOManagerT& io, const int idx) {
    Node left_child = get_child(io, idx - 1);
    Node right_child = get_child(io, idx + 1);

    // If the left child has >= (T - 1) keys, borrow a key from it
    if (idx != 0 && left_child.used_keys >= t) {
        borrow_from_prev_node(io, idx);

    // If the right child has >= (T - 1) keys, borrow a key from it
    } else if (idx != used_keys && right_child.used_keys >= t) {
        borrow_from_next_node(io, idx);

    // Merge child[idx] with its sibling
    // - if (child[idx] isn't the last child) {
    //       merge it with (idx)-th child
    //  } else {
    //       merge with (idx)-th child which now has at least T keys
    //  }
    } else {
        int merge_index = (idx != used_keys) ? idx : idx - 1;
        if (merge_index >= 0)
            merge_node(io, merge_index);
    }
}

template<class K, class V>
void BTree<K,V>::BTreeNode::borrow_from_prev_node(IOManagerT& io, const int idx) {
    // To borrow a key from child[idx-1] and insert it to child[idx]
    Node prev = get_child(io, idx - 1);
    Node child = get_child(io, idx);

    // Move keys and children
    shift_right_by_one(child.key_pos, child.used_keys, 0);
    if (!child.is_leaf())
        shift_right_by_one(child.child_pos, child.used_keys + 1, 0);

    // Set CURR's key_pos to the first CHILD's key_pos
    child.key_pos[0] = key_pos[idx - 1];

    // Set PREV's last child_pos to the first CHILD's child_pos
    if (!child.is_leaf())
        child.child_pos[0] = prev.child_pos[prev.used_keys];

    // Set PREV's key_pos to CURR's key_pos
    key_pos[idx - 1] = prev.key_pos[prev.used_keys - 1];

    child.used_keys++;
    prev.used_keys--;

    io.write_node(child, child.m_pos);
    io.write_node(prev, prev.m_pos);
    io.write_node(*this, m_pos);
}

template<class K, class V>
void BTree<K,V>::BTreeNode::borrow_from_next_node(IOManagerT& io, const int idx) {
    Node child = get_child(io, idx);
    Node next = get_child(io, idx + 1);

    // Set CURR's key_pos to the last CHILD's key_pos
    child.key_pos[child.used_keys] = key_pos[idx];

    //  Set NEXT's first child to the last CHILD's child_pos
    if (!child.is_leaf())
        child.child_pos[child.used_keys + 1] = next.child_pos[0];

    // Set the first NEXT's key to CURR's key_pos
    key_pos[idx] = next.key_pos[0];

    // Move keys and children
    shift_left_by_one(next.key_pos, 1, next.used_keys);
    if (!next.is_leaf())
        shift_left_by_one(next.child_pos, 1, next.used_keys + 1);

    child.used_keys++;
    next.used_keys--;

    io.write_node(child, child.m_pos);
    io.write_node(next, next.m_pos);
    io.write_node(*this, m_pos);
}

template<class K, class V>
void BTree<K,V>::BTreeNode::traverse(IOManagerT& io) {
    int i;
    Node node;

    for (i = 0; i < used_keys; ++i) {
        if (!is_leaf()) {
            node = get_child(io, i);
//            cout << endl;
            node.traverse(io);
//            cout << endl;
        }
//        entry = read_entry(io, i);
//        cout << "[key]: " << entry->key << " - [value]: " << entry->value << " ";
    }
    if (!is_leaf()) {
        node = get_child(io, i);
//        cout << endl;
        node.traverse(io);
//        cout << endl;
    }
}
