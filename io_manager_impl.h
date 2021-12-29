#pragma once

template <typename K, typename V>
IOManager<K, V>::IOManager(const std::string& path, const int16_t user_t) : t(user_t), file(path, 0)  {}

template <typename K, typename V>
bool IOManager<K,V>:: is_ready() {
    return !file.isEmpty();
}

template <typename K, typename V>
int32_t IOManager<K,V>::get_node_size_in_bytes(Node& node) {
    return
        sizeof (node.used_keys) +           // 2 bytes
        sizeof (node.flag) +            //  1 byte
        node.key_pos.size() * sizeof(K) +
        node.child_pos.size() * sizeof(K);
}

template <typename K, typename V>
void IOManager<K,V>::write_entry(EntryT && entry, const int32_t pos)    {
    file.set_pos(pos);

    uint8_t flag = 1;
    file.write_next(flag);
    file.write_next(entry.key);
    file.write_next(entry.value);
}

template <typename K, typename V>
Entry<K, V> IOManager<K,V>::read_entry(const int32_t pos) {
        file.set_pos(pos);

        uint8_t flag = file.read_byte();
        K key = file.read_next<K>();
        V value = file.read_next<V>();
        return { key, value };
    }

template <typename K, typename V>
void IOManager<K,V>::write_flag(uint8_t flag, const int32_t pos) {
    file.set_pos(pos);

    file.write_next(flag);
}

template <typename K, typename V>
int32_t IOManager<K,V>::read_header() {
    file.set_pos(0);

    auto t_from_file = file.read_int16();
    if (t != t_from_file)
        throw std::logic_error("Wrong tree order is used for file: PATH");
//        auto key_size = file.read_next<uint8_t>();
//        auto curr_value_type = file.read_next<uint8_t>();
//        auto cur_value_type_size = file.read_next<uint8_t>();
//
//        assert(key_size == sizeof(K));
//        assert(curr_value_type == value_type<V>());
//        assert(cur_value_type_size == value_type_size<V>());

    int32_t posRoot = file.read_int32();
    return posRoot;
}

template <typename K, typename V>
int32_t IOManager<K,V>::write_header() {
    file.set_pos(0);
//        uint8_t val = value_type_size<V>();
//        uint8_t i = value_type<V>();
//        assert((sizeof(t) + 1 + i + val) == 5);
//        const int root_pos = 6;

    file.write_next(t);                             // 2 bytes
//        file.write_next<uint8_t>(sizeof(K));            // 1 byte
//        file.write_next<uint8_t>(i);                    // 1 byte
//        file.write_next<uint8_t>(val);                      // 1 byte
    file.write_next(6);                      // 4 byte -> write root pos
    return file.get_pos();
}

template <typename K, typename V>
void IOManager<K,V>::write_new_pos_for_root_node(const int32_t posRoot) {
    file.set_pos(ROOT_POS_IN_HEADER);

    file.write_next(posRoot);
}

template <typename K, typename V>
void IOManager<K,V>::write_invalidated_root() {
    file.set_pos(ROOT_POS_IN_HEADER);

    file.write_next(INVALID_ROOT_POS);
    file.shrink_to_fit();
}

template <typename K, typename V>
int32_t IOManager<K,V>::write_node(const Node& node, const int32_t pos) {
    file.set_pos(pos);

    file.write_next(node.flag);
    file.write_next(node.used_keys);
    file.write_node_vector(node.key_pos);
    file.write_node_vector(node.child_pos);
    return file.get_pos();
}

template <typename K, typename V>
typename BTree<K,V>::Node IOManager<K,V>::read_node(const int32_t pos) {
    Node node(t, false);
    read_node(&node, pos);
    return node;
}

template <typename K, typename V>
void IOManager<K,V>::read_node(Node* node, const int32_t pos) {
    file.set_pos(pos);

    node->m_pos = pos;
    node->flag = file.read_byte();
    node->used_keys = file.read_int16();
    file.read_node_vector(node->key_pos);
    file.read_node_vector(node->child_pos);
}

template <typename K, typename V>
int32_t IOManager<K,V>::get_file_pos_end() {
    file.set_file_pos_to_end();
    return file.get_pos();
}