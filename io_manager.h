#include "file_mapping.h"
#include "entry.h"
#include "btree.h"

template <typename K, typename V>
class IOManager {
    using EntryT = Entry<K,V>;
    using Node = typename BTree<K,V>::Node;

    const int16_t t = 0;
    MappedFile file;

    static constexpr uint8_t ROOT_POS_IN_HEADER = sizeof(t);
    static constexpr int64_t INITIAL_ROOT_POS_IN_HEADER = sizeof(t) + sizeof(int64_t);
public:
    static constexpr int64_t INVALID_ROOT_POS = -1;

    IOManager(const std::string& path, const int16_t user_t);

    bool is_ready();

    int32_t get_node_size_in_bytes(Node& node);

    void write_entry(EntryT && entry, const int64_t pos);
    EntryT read_entry(const int64_t pos);

    void write_flag(uint8_t flag, const int64_t pos);

    int64_t read_header();
    int64_t write_header();

    void write_invalidated_root();
    void write_new_pos_for_root_node(const int64_t posRoot);

    int64_t write_node(const Node& node, const int64_t pos);

    Node read_node(const int64_t pos);
    void read_node(Node* node, const int64_t pos);

    int64_t get_file_pos_end();
};

#include "io_manager_impl.h"