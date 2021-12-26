#include "file_mapping.h"
#include "entry.h"
#include "btree.h"

/**
 * Storage structures:
 *
 * - Header (6 bytes):
 *     - T                        |=> takes 2 bytes -> tree degree
 *     - KEY_SIZE                 |=> takes 1 byte
 *     - VALUE_TYPE               |=> takes 1 byte ->  VALUE_TYPE = 0 for primitives: (u)int32_t, (u)int64_t, float, double
 *                                                     VALUE_TYPE = 1 for container of values: (w)string, vectors<T>
 *                                                     VALUE_TYPE = 2 for blob
 *
 *     - ELEMENT_SIZE             |=> takes 1 byte  -> ELEMENT_SIZE = sizeof(VALUE_TYPE) for primitives
 *                                                     ELEMENT_SIZE = mask from the 8 bits (max 256 bytes) for non-primitives
 *     - ROOT POS                 |=> takes 8 bytes -> pos in file
 *
 * - Node (N bytes):
 *     - FLAG                     |=> takes 1 byte                 -> for "is_deleted" or "is_leaf"
 *     - USED_KEYS                |=> takes 2 bytes                -> for the number of "active" keys in the node
 *     - KEY_POS                  |=> takes (2 * t - 1) * KEY_SIZE -> for key positions in file
 *     - CHILD_POS                |=> takes (2 * t) * KEY_SIZE     -> for key positions in file
 *
 * - Entry (M bytes):
 *     - KEY                      |=> takes KEY_SIZE bytes (4 bytes is enough for 10^8 different keys)
 *     ----------–-----
 *        - VALUE                 |=> takes ELEMENT_SIZE bytes for primitive VALUE_TYPE
 *     or
 *        - NUMBER_OF_ELEMENTS    |=> takes 4 bytes
 *        - VALUES                |=> takes (ELEMENT_SIZE * NUMBER_OF_ELEMENTS) bytes
 *     ----------–-----
*/

template <typename K, typename V>
struct IOManager {
    static constexpr int64_t INVALID_ROOT_POS = -1;

    using EntryT = Entry<K,V>;
    using Node = typename BTree<K,V>::Node;

    MappedFile file;

    explicit IOManager(const std::string& path) : file(path, 0) {}

    bool is_ready() {
        return !file.isEmpty();
    }

    void write_entry(const EntryT& entry, const int64_t pos)    {
        file.set_pos(pos);

        file.write_next(entry.key);
        file.write_next(entry.value);
    }

    K read_key(const int64_t pos) {
        file.set_pos(pos);

        K key = file.read_next<K>();
        assert(key > -1);
        return key;
    }

    EntryT read_entry(const int64_t pos) {
        file.set_pos(pos);

        K key = file.read_next<K>();
        V value = file.read_next<V>();
        assert(key > -1);
        return { key, value };
    }


    void write_flag(uint8_t flag, const int64_t pos) {
        file.set_pos(pos);

        file.write_next(flag);
    }

    int64_t read_header(const int16_t& user_t) {
        file.set_pos(0);

        auto t = file.read_next<int16_t>();
        validate(t == user_t,
                 "The sizeof(KEY) for your tree doesn't equal to the KEY used in storage: ");
//
//        auto key_size = file.read_next<uint8_t>();
//        validate(key_size == sizeof(K),
//                 "The sizeof(KEY) for your tree doesn't equal to the KEY used in storage: ");
//
//        auto element_size = file.read_next<uint8_t>();
//        validate(element_size == get_element_size<V>(),
//            "The sizeof(KEY) for your tree doesn't equal to the KEY used in storage: ");
//
//        auto value_type_code = file.read_next<uint8_t>();
//        validate(value_type_code == get_value_type_code<V>(),
//            "The sizeof(KEY) for your tree doesn't equal to the KEY used in storage: ");

        auto posRoot = file.read_next<int64_t>();
        return posRoot;
    }

    int64_t write_header(const int16_t t) {
        file.set_pos(0);

//        uint8_t element_size = get_element_size<V>();
//        assert(element_size > 0);
//        uint8_t value_type_code = get_value_type_code<V>();

        file.write_next(t);                             // 2 bytes
//        file.write_next<uint8_t>(sizeof(K));            // 1 byte
//        file.write_next<uint8_t>(element_size);         // 1 byte
//        file.write_next<uint8_t>(value_type_code);      // 1 byte
        file.write_next<int64_t>(10);                   // 8 byte -> write root pos
        return file.get_pos();
    }

    void writeUpdatePosRoot(const int64_t posRoot) {
        file.set_pos(2);

        file.write_next(posRoot);
    }

    int64_t write_node(const Node& node, const int64_t pos) {
        file.set_pos(pos);

        file.write_next(node.flag);
        file.write_next(node.used_keys);
        file.write_next(node.key_pos);
        file.write_next(node.child_pos);
        return file.get_pos();
    }

    void read_node(Node* node, const int64_t pos) {
        file.set_pos(pos);

        node->m_pos = pos;
        node->flag = file.read_byte();
        node->used_keys = file.read_next<int16_t>();
        file.read_vector(node->key_pos);
        file.read_vector(node->child_pos);
    }

    int64_t get_file_pos_end() {
        file.set_file_pos_to_end();
        return file.get_pos();
    }

private:
    void validate(bool expression, const std::string& msg) {
        if (!expression) {
            throw std::logic_error(msg + " in " + file.path);
        }
    }
};