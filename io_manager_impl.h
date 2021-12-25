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

    void write_entry(const EntryT& entry, const int pos)    {
        file.set_pos(pos);

        file.write_next(entry.key);
        file.write_next(entry.value);
    }

    K read_key(const int pos) {
        file.set_pos(pos);
        //        assert(key > -1);
        return file.read_next<K>();
    }

    EntryT read_entry(const int pos) {
        file.set_pos(pos);

//    readDisk->readByte((uint8_t&) flag);
//    readDisk->readBytes(reinterpret_cast<uint8_t*> (&lenKey), 0, sizeof (lenKey));
//    key.resize(lenKey);
//    readDisk->readBytes((uint8_t*) &key, 0, 4);
//    readDisk->readBytes(reinterpret_cast<uint8_t*> (&lenValue), 0, sizeof (lenValue));
//    value.resize(lenValue);
//    readDisk->readBytes((uint8_t*) &value, 0, 4);

        K key = file.read_next<K>();
        V value = file.read_next<V>();
        assert(key > -1);
        return { key, value };
    }


    void write_flag(char flag, const int pos) {
        file.set_pos(pos);

        file.write_byte(flag);
    }

    int read_header(int& t) {
        file.set_pos(0);

        t = file.read_int();
        int posRoot = file.read_int();
        return posRoot;
    }

    void write_header(const int t, const int posRoot) {
        file.set_pos(0);

        file.write_int(t);
        file.write_int(posRoot);
    }

    void writeUpdatePosRoot(const int posRoot) {
        file.set_pos(4);

        file.write_int(posRoot);
    }

    int write_node(const Node& node, const int pos) {
        file.set_pos(pos);

        file.write_byte(node.flag);
        file.write_int(node.used_keys);
        file.write_vector(node.key_pos);
        file.write_vector(node.child_pos);
        return file.get_pos();
    }

    void read_node(Node* node, const int pos) {
        file.set_pos(pos);

        node->m_pos = pos;
        node->flag = file.read_byte();
        node->used_keys = file.read_int();
        file.read_vector(node->key_pos);
        file.read_vector(node->child_pos);
    }

    int get_file_pos_end() {
        file.set_file_pos_to_end();
        return file.get_pos();
    }
};