#pragma once

#include "utils/utils.h"
#include "utils/error.h"

namespace btree {
    template <typename K, typename V>
    IOManager<K, V>::IOManager(const std::string& path, const int16_t user_t) : t(user_t), file(path, 0) {}

    template <typename K, typename V>
    int64_t IOManager<K, V>::write_header() {
        auto region = file.get_mapped_region(0);

        file.write_next_primitive(region, t);
        file.template write_next_primitive<uint8_t>(region, sizeof(K)); // todo: replace with KEY type enum (because of variable key length)?
        file.template write_next_primitive<uint8_t>(region, get_value_type_code<V>());
        file.template write_next_primitive<uint8_t>(region, get_element_size<V>());
        file.write_next_primitive(region, INITIAL_ROOT_POS_IN_HEADER);
        return file.get_pos();
    }

    template <typename K, typename V>
    int64_t IOManager<K, V>::read_header() {
        auto pRegion = file.get_mapped_region(0);

        auto t_from_file = file.read_int16(pRegion.get());
        validate(t == t_from_file, error_msg::wrong_order_msg, file.path);

        auto key_size = file.read_byte(pRegion.get()); // todo:  replace with KEY type enum (because of variable key length)?
        validate(key_size == sizeof(K), error_msg::wrong_key_size_msg, file.path);

        auto value_type_code = file.read_byte(pRegion.get());
        validate(value_type_code == get_value_type_code<V>(), error_msg::wrong_value_type_msg, file.path);

        auto element_size = file.read_byte(pRegion.get());
        validate(element_size == get_element_size<V>(), error_msg::wrong_element_size_msg, file.path);

        auto posRoot = file.read_int32(pRegion.get());
        return posRoot;
    }

    template <typename K, typename V>
    bool IOManager<K, V>::is_ready() const {
        return !file.is_empty();
    }

    template <typename K, typename V>
    void IOManager<K, V>::write_entry(const EntryT& e, const int64_t pos) {
        int32_t key_size = e.key.size() * sizeof(typename K::value_type);
        int32_t data_size = e.size_in_bytes;
        file.get_mapped_region(pos);

        file.write_next_data(e.key.data(), key_size);
        file.write_next_data(e.data, data_size);
    }

    template <typename K, typename V>
    typename BTree<K,V>::EntryT IOManager<K, V>::read_entry(const int64_t pos) {
        const auto& ptr = file.get_mapped_region(pos);

        auto [data, len] = file.read_next_data<const uint8_t*>(ptr.get());
        K key((const char*)data, len);
        auto [value, size] = file.template read_next_data<typename EntryT::ValueType>(ptr.get());
        return { key, value, size };
    }

    template <typename K, typename V>
    K IOManager<K, V>::read_key(const int64_t pos) {
        const auto& ptr = file.get_mapped_region(pos);

        auto [data, len] = file.read_next_data<const uint8_t*>(ptr.get());
        K key((const char*)data, len);
        return key;
    }

    template <typename K, typename V>
    void IOManager<K, V>::write_new_pos_for_root_node(const int64_t posRoot) {
        auto region = file.get_mapped_region(ROOT_POS_IN_HEADER);

        file.write_next_primitive(region, posRoot);
    }

    template <typename K, typename V>
    void IOManager<K, V>::write_invalidated_root() {
        auto region = file.get_mapped_region(ROOT_POS_IN_HEADER);

        file.write_next_primitive(region, INVALID_POS);
        file.shrink_to_fit();
    }

    template <typename K, typename V>
    int64_t IOManager<K, V>::write_node(const Node& node, const int64_t pos) {
        auto ptr = file.get_mapped_region(pos);

        file.write_next_primitive(ptr, node.is_leaf);
        file.write_next_primitive(ptr, node.used_keys);
        file.write_node_vector(node.key_pos);
        file.write_node_vector(node.child_pos);
        return file.get_pos();
    }

    template <typename K, typename V>
    BTreeNode <K, V> IOManager<K, V>::read_node(const int64_t pos) {
        const auto& ptr = file.get_mapped_region(pos);

        Node node(t, false);
        node.m_pos = pos;
        node.is_leaf = file.read_byte(ptr.get());
        node.used_keys = file.read_int16(ptr.get());
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