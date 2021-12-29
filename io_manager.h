#include "file_mapping.h"
#include "entry.h"
#include "btree.h"

template <typename K, typename V>
class IOManager {
    using EntryT = Entry<K,V>;
    using Node = typename BTree<K,V>::Node;

    const int16_t t = 0;
    MappedFile file;
public:
    static constexpr int32_t INVALID_ROOT_POS = -1;

    IOManager(const std::string& path, const int16_t user_t);

    bool is_ready();

    int32_t get_node_size_in_bytes(Node& node);

    void write_entry(EntryT && entry, const int32_t pos);
    EntryT read_entry(const int32_t pos);

    void write_flag(uint8_t flag, const int32_t pos);

    int32_t read_header();
    int32_t write_header();

    void writeUpdatePosRoot(const int32_t posRoot);

    int32_t write_node(const Node& node, const int32_t pos);

    Node read_node(const int32_t pos);
    void read_node(Node* node, const int32_t pos);

    int32_t get_file_pos_end();
    void shrink_to_fit();
};

#include "io_manager_impl.h"