#include "file_mapping.h"
#include "entry.h"
#include "btree.h"

template <typename K, typename V>
struct IOManager {
    using EntryT = Entry<K,V>;
    using Node = typename BTree<K,V>::Node;

    MappedFile file;

    explicit IOManager(const std::string& path) : file(path, 0) {}

    bool is_ready() {
        return !file.isEmpty();
    }

    int get_node_size_in_bytes(Node& node) {
        return
            sizeof (node.used_keys) +           // 4/8 bytes
            sizeof (node.used_keys) +            //  1 byte
            node.arrayPosKey.size() * sizeof(K) +
            node.arrayPosChild.size() * sizeof(K);
    }

    void write_entry(const EntryT& entry, const int pos)    {
        file.set_pos(pos);

        char flag = 1;
        file.write_byte(flag);
        file.write_next(entry.key);
        file.write_next(entry.value);
    }

    void read_entry(EntryT& entry, const int pos) {
        file.set_pos(pos);

//    readDisk->readByte((uint8_t&) flag);
//    readDisk->readBytes(reinterpret_cast<uint8_t*> (&lenKey), 0, sizeof (lenKey));
//    key.resize(lenKey);
//    readDisk->readBytes((uint8_t*) &key, 0, 4);
//    readDisk->readBytes(reinterpret_cast<uint8_t*> (&lenValue), 0, sizeof (lenValue));
//    value.resize(lenValue);
//    readDisk->readBytes((uint8_t*) &value, 0, 4);

        char flag = file.read_byte();
        entry.key = file.read_next<K>();
        entry.value = file.read_next<V>();
    }


    void write_flag(char flag, const int pos) {
        file.set_pos(pos);

        file.write_byte(flag);
    }

    void read_header(int& t, int& posRoot) {
        file.set_pos(0);

        t = file.read_int();
        posRoot = file.read_int();
    }

    int write_header(const int t, const int posRoot) {
        file.set_pos(0);

        file.write_int(t);
        file.write_int(posRoot);
        return file.get_pos();
    }

    void writeUpdatePosRoot(const int posRoot) {
        file.set_pos(4);

        file.write_int(posRoot);
    }

    int write_node(const Node& node, const int pos) {
        file.set_pos(pos);

        file.write_byte(node.flag);
        file.write_int(node.used_keys);
        file.write_vector(node.arrayPosKey);
        file.write_vector(node.arrayPosChild);
        return file.get_pos();
    }

    void read_node(Node* node, const int pos) {
        file.set_pos(pos);

        node->m_pos = pos;
        node->flag = file.read_byte();
        node->used_keys = file.read_int();
        file.read_vector(node->arrayPosKey);
        file.read_vector(node->arrayPosChild);
    }

    int get_file_pos_end() {
        file.set_file_pos_to_end();
        return file.get_pos();
    }
};