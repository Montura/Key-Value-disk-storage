#pragma once

#include "utils/utils.h"

namespace btree {
    template <typename K, typename V>
    IOManager<K, V>::IOManager(const std::string& path, const int16_t user_t) : t(user_t), file(path, 0) {}

    template <typename K, typename V>
    int64_t IOManager<K, V>::write_header() {
        file.set_pos(0);

        file.write_next_primitive(t);
        file.write_next_primitive<uint8_t>(sizeof(K));
        file.write_next_primitive<uint8_t>(get_value_type_code<V>());
        file.write_next_primitive<uint8_t>(get_element_size<V>());
        file.write_next_primitive(INITIAL_ROOT_POS_IN_HEADER);
        return file.get_pos();
    }

    template <typename K, typename V>
    int64_t IOManager<K, V>::read_header() {
        file.set_pos(0);

        auto t_from_file = file.read_int16();
        validate(t == t_from_file,
                 "The order(T) for your tree doesn't equal to the order(T) used in storage: " + file.path);

        auto key_size = file.read_byte();
        validate(key_size == sizeof(K),
                 "The sizeof(KEY) for your tree doesn't equal to the sizeof(KEY) used in storage: " + file.path);

        auto value_type_code = file.read_byte();
        validate(value_type_code == get_value_type_code<V>(),
                 "The VALUE_TYPE for your tree doesn't equal to the VALUE_TYPE used in storage: " + file.path);

        auto element_size = file.read_byte();
        validate(element_size == get_element_size<V>(),
                 "The ELEMENT_SIZE for your tree doesn't equal to the ELEMENT_SIZE used in storage: " + file.path);

        auto posRoot = file.read_int32();
        return posRoot;
    }

    template <typename K, typename V>
    bool IOManager<K, V>::is_ready() const {
        return !file.is_empty();
    }

    template <typename K, typename V>
    void IOManager<K, V>::write_entry(const EntryT& e, const int64_t pos) {
        file.set_pos(pos);

        file.write_next_primitive(e.key);
        file.write_next_data(e.data, e.size_in_bytes);
    }

    template <typename K, typename V>
    typename BTree<K,V>::EntryT IOManager<K, V>::read_entry(const int64_t pos) {
        file.set_pos(pos);

        K key = file.read_next_primitive<K>();
        auto [value, size] = file.read_next_data<typename EntryT::ValueType>();
        return { key, value, size };
    }

    template <typename K, typename V>
    K IOManager<K, V>::read_key(const int64_t pos) {
        file.set_pos(pos);

        return file.read_next_primitive<K>();
    }

    template <typename K, typename V>
    void IOManager<K, V>::write_new_pos_for_root_node(const int64_t posRoot) {
        file.set_pos(ROOT_POS_IN_HEADER);

        file.write_next_primitive(posRoot);
    }

    template <typename K, typename V>
    void IOManager<K, V>::write_invalidated_root() {
        file.set_pos(ROOT_POS_IN_HEADER);

        file.write_next_primitive(INVALID_POS);
        file.shrink_to_fit();
    }

    template <typename K, typename V>
    int64_t IOManager<K, V>::write_node(const Node& node, const int64_t pos) {
        file.set_pos(pos);

        file.write_next_primitive(node.is_leaf);
        file.write_next_primitive(node.used_keys);
        file.write_node_vector(node.key_pos);
        file.write_node_vector(node.child_pos);
        return file.get_pos();
    }

    template <typename K, typename V>
    BTreeNode <K, V> IOManager<K, V>::read_node(const int64_t pos) {
        file.set_pos(pos);

        Node node(t, false);
        node.m_pos = pos;
        node.is_leaf = file.read_byte();
        node.used_keys = file.read_int16();
        file.read_node_vector(node.key_pos);
        file.read_node_vector(node.child_pos);
        return node;
    }

    template <typename K, typename V>
    int64_t IOManager<K, V>::get_file_pos_end() {
        file.set_file_pos_to_end();
        return file.get_pos();
    }
}