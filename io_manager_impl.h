#pragma once

template<typename K, typename V>
IOManager<K, V>::IOManager(const std::string &path) : file(path, 0) {}

template<typename K, typename V>
int64_t IOManager<K, V>::write_header(const int16_t t) {
    file.set_pos(0);

    uint8_t element_size = get_element_size<V>();
    assert(element_size > 0);
    uint8_t value_type_code = get_value_type_code<V>();

    file.write_next(t);                             // 2 bytes
    file.write_next<uint8_t>(sizeof(K));            // 1 byte
    file.write_next<uint8_t>(element_size);         // 1 byte
    file.write_next<uint8_t>(value_type_code);      // 1 byte
    file.write_next<int64_t>(HEADER_SIZE - 1);      // 8 byte -> write root pos
    return file.get_pos();
}

template<typename K, typename V>
int64_t IOManager<K, V>::read_header(const int16_t &user_t) {
    file.set_pos(0);

    auto t = file.read_next<int16_t>();
    validate(t == user_t,
             "The sizeof(KEY) for your tree doesn't equal to the KEY used in storage: ");

    auto key_size = file.read_next<uint8_t>();
    validate(key_size == sizeof(K),
             "The sizeof(KEY) for your tree doesn't equal to the KEY used in storage: ");

    auto element_size = file.read_next<uint8_t>();
    validate(element_size == get_element_size<V>(),
             "The sizeof(KEY) for your tree doesn't equal to the KEY used in storage: ");
//
    auto value_type_code = file.read_next<uint8_t>();
    validate(value_type_code == get_value_type_code<V>(),
             "The sizeof(KEY) for your tree doesn't equal to the KEY used in storage: ");

    auto posRoot = file.read_next<int64_t>();
    return posRoot;
}

template<typename K, typename V>
int64_t IOManager<K, V>::write_node(const Node &node, const int64_t pos) {
    file.set_pos(pos);

    file.write_next(node.flag);
    file.write_next(node.used_keys);
    file.write_next(node.key_pos);
    file.write_next(node.child_pos);
    return file.get_pos();
}

template<typename K, typename V>
void IOManager<K, V>::read_node(Node *node, const int64_t pos) {
    file.set_pos(pos);

    node->m_pos = pos;
    node->flag = file.read_byte();
    node->used_keys = file.read_next<int16_t>();
    file.read_vector(node->key_pos);
    file.read_vector(node->child_pos);
}


template<typename K, typename V>
void IOManager<K, V>::write_entry(const EntryT &entry, const int64_t pos) {
    file.set_pos(pos);

    file.write_next(entry.key);
    file.write_next(entry.value);
}

template<typename K, typename V>
Entry<K, V> IOManager<K, V>::read_entry(const int64_t pos) {
    file.set_pos(pos);

    K key = file.read_next<K>();
    V value = file.read_next<V>();
    assert(key > -1);
    return {key, value};
}

template<typename K, typename V>
K IOManager<K, V>::read_key(const int64_t pos) {
    file.set_pos(pos);

    K key = file.read_next<K>();
    assert(key > -1);
    return key;
}

template<typename K, typename V>
void IOManager<K, V>::write_flag(uint8_t flag, const int64_t pos) {
    file.set_pos(pos);

    file.write_next(flag);
}

template<typename K, typename V>
void IOManager<K, V>::write_new_pos_for_root_node(const int64_t posRoot) {
    file.set_pos(ROOT_POS_IN_HEADER);

    file.write_next(posRoot);
}

template<typename K, typename V>
bool IOManager<K, V>::is_ready() {
    return !file.isEmpty();
}

template<typename K, typename V>
int64_t IOManager<K, V>::get_file_pos_end() {
    file.set_file_pos_to_end();
    return file.get_pos();
}

template<typename K, typename V>
void IOManager<K, V>::validate(bool expression, const std::string &msg) {
    if (!expression) {
        throw std::logic_error(msg + " in " + file.path);
    }
}