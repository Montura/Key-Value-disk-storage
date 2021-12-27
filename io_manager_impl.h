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
        char flag = 1;
        K strKey = entry.key;
        V strValue = entry.value;

        file.setPosFile(pos);
        file.write_byte(flag);
        file.write_next(strKey);
        file.write_next(strValue);
    }

    void read_entry(EntryT& entry, const int pos) {
//    int lenKey, lenValue;

        file.setPosFile(pos);

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
        assert(flag > -1);
        assert(entry.key > -1);
    }


    void write_flag(char flag, const int pos) {
        file.setPosFile(pos);
        file.write_byte(flag);
    }

    void read_header(int& t, int& posRoot) {
        file.setPosFile(0);
        t = file.read_int();
        posRoot = file.read_int();
    }

    void write_header(const int t, const int posRoot) {
        file.setPosFile(0);

        file.write_int(t);
        file.write_int(posRoot);
    }

    void writeUpdatePosRoot(const int posRoot) {
        file.setPosFile(4);

        file.write_int(posRoot);
    }

    void write_node(const Node& node, const int pos) {
        file.setPosFile(pos);

        file.write_byte(node.flag);
        file.write_int(node.used_keys);
        file.write_vector(node.arrayPosKey);
        file.write_vector(node.arrayPosChild);
    }

    void read_node(Node* node, const int pos) {
        file.setPosFile(pos);

        node->m_pos = pos;
        node->flag = file.read_byte();
        node->used_keys = file.read_int();
        file.read_vector(node->arrayPosKey);
        file.read_vector(node->arrayPosChild);
    }

    int getPosFile() {
        return file.getPosFile();
    }

    void setPosEndFile() {
        file.setPosEndFile();
    }
};