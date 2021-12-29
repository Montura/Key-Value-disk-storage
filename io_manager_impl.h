#include "file_mapping.h"
#include "entry.h"
#include "btree.h"

template <typename K, typename V>
class IOManager {
    using EntryT = Entry<K,V>;
    using Node = typename BTree<K,V>::Node;

    MappedFile file;
    const int t = 0;
public:
    static constexpr int INVALID_ROOT_POS = -1;

    explicit IOManager(const std::string& path, const int user_t) : file(path, 0), t(user_t) {}

    bool is_ready() {
        return !file.isEmpty();
    }

    int get_node_size_in_bytes(Node& node) {
        return
            sizeof (node.used_keys) +           // 4/8 bytes
            sizeof (node.flag) +            //  1 byte
            node.key_pos.size() * sizeof(K) +
            node.child_pos.size() * sizeof(K);
    }

    void write_entry(EntryT && entry, const int pos)    {
        file.set_pos(pos);

        char flag = 1;
        file.write_next(flag);
        file.write_next(entry.key);
        file.write_next(entry.value);
    }

    EntryT read_entry(const int pos) {
        file.set_pos(pos);

        char flag = file.read_byte();
        K key = file.read_next<K>();
        V value = file.read_next<V>();
        return { key, value };
    }


    void write_flag(char flag, const int pos) {
        file.set_pos(pos);

        file.write_next(flag);
    }

    int read_header() {
        file.set_pos(0);

        auto t_from_file = file.read_int();
        if (t != t_from_file)
            throw std::logic_error("Wrong tree order is used for file: PATH");
//        auto key_size = file.read_next<uint8_t>();
//        auto curr_value_type = file.read_next<uint8_t>();
//        auto cur_value_type_size = file.read_next<uint8_t>();
//
//        assert(key_size == sizeof(K));
//        assert(curr_value_type == value_type<V>());
//        assert(cur_value_type_size == value_type_size<V>());

        int posRoot = file.read_int();
        return posRoot;
    }

    int write_header() {
        file.set_pos(0);
//        uint8_t val = value_type_size<V>();
//        uint8_t i = value_type<V>();
//        assert((sizeof(t) + 1 + i + val) == 5);
//        const int root_pos = 6;

        file.write_next(t);                               // 2 bytes
//        file.write_next<uint8_t>(sizeof(K));            // 1 byte
//        file.write_next<uint8_t>(i);                    // 1 byte
//        file.write_next<uint8_t>(val);                      // 1 byte
        file.write_next(8);                      // 4 byte -> write root pos
        return file.get_pos();
    }

    void writeUpdatePosRoot(const int posRoot) {
        file.set_pos(4);

        file.write_next(posRoot);
    }

    void shrink_to_fit() {
        file.shrink_to_fit();
    }

    int write_node(const Node& node, const int pos) {
        file.set_pos(pos);

        file.write_next(node.flag);
        file.write_next(node.used_keys);
        file.write_node_vector(node.key_pos);
        file.write_node_vector(node.child_pos);
        return file.get_pos();
    }

    Node read_node(const int pos) {
        Node node(t, false);
        read_node(&node, pos);
        return node;
    }

    void read_node(Node* node, const int pos) {
        file.set_pos(pos);

        node->m_pos = pos;
        node->flag = file.read_byte();
        node->used_keys = file.read_int();
        file.read_node_vector(node->key_pos);
        file.read_node_vector(node->child_pos);
    }

    int get_file_pos_end() {
        file.set_file_pos_to_end();
        return file.get_pos();
    }
};