#pragma once

#include "mapped_file.h"
#include "utils/forward_decl.h"

/**
 * Storage structures:
 *
 * - Header (6 bytes):
 *     - T                        |=> takes 2 bytes -> tree degree
 *     - KEY_SIZE                 |=> takes 1 byte
 *     - VALUE_TYPE               |=> takes 1 byte ->  VALUE_TYPE = 0 for integer primitives: int32_t, int64_t
 *                                                     VALUE_TYPE = 1 for unsigned integer primitives: uint32_t, uint64_t
 *                                                     VALUE_TYPE = 2 for floating-point primitives: float, double
 *                                                     VALUE_TYPE = 3 for container of values: (w)string
 *                                                     VALUE_TYPE = 4 for blob
 *
 *     - ELEMENT_SIZE             |=> takes 1 byte  -> ELEMENT_SIZE = sizeof(VALUE_TYPE) for primitives
 *                                                     ELEMENT_SIZE = sizeof(VALUE_SUBTYPE) for containers or blob
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
namespace btree {
    template <typename K, typename V>
    class IOManager {
        using Node = BTreeNode<K, V>;
        using EntryT = typename BTree<K, V>::EntryT;

        const int16_t t = 0;
        MappedFile<K,V> file;

        static constexpr uint8_t ROOT_POS_IN_HEADER = sizeof(t) + 3;
    public:
        static constexpr int64_t INITIAL_ROOT_POS_IN_HEADER = ROOT_POS_IN_HEADER + sizeof(int64_t);
        static constexpr int64_t INVALID_POS = -1;

        IOManager(const std::string& path, const int16_t user_t);

        bool is_ready() const;

        int64_t write_node(const Node& node, const int64_t pos);
        void write_entry(const EntryT& e, const int64_t pos);

        Node read_node(const int64_t pos);
        EntryT read_entry(const int64_t pos);
        K read_key(const int64_t pos);

        int64_t read_header();
        int64_t write_header();

        void write_invalidated_root();
        void write_new_pos_for_root_node(const int64_t posRoot);

        int64_t get_file_pos_end();
    };
}
#include "io_manager_impl.h"