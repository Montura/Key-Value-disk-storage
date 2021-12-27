#include <string>
#include <fstream>
#include "boost_include.h"

struct MappedFile {
    MappedFile(const std::string& fn, int64_t bytes_num);
    ~MappedFile();

    template <typename T>
    T read_next() {
        static_assert(std::is_arithmetic_v<T>);
        char *value_begin = mapped_region_begin + m_pos;
        m_pos += sizeof(T);
        return *(reinterpret_cast<T*>(value_begin));
    }

    template <typename T>
    void write_next(T val) {
        static_assert(std::is_arithmetic_v<T>);
        m_pos = write_to_dst(val, m_pos);
        m_capacity = std::max(m_pos, m_capacity);
    }

    template <typename T>
    void write_vector(const std::vector<T>& vec) {
        int64_t total_size = static_cast<int64_t>(sizeof(T)) * vec.size();
        if (m_pos + total_size > m_size) {
            resize(std::max(2 * m_size, total_size));
        }
        const char* data = reinterpret_cast<const char *>(vec.data());
        std::copy(data, data + total_size, mapped_region_begin + m_pos);
        m_pos += total_size;
        m_capacity = std::max(m_pos, m_capacity);
    }

    template <typename T>
    void read_vector(std::vector<T>& vec) {
        uint32_t total_size = sizeof(T) * vec.size();
//    assert(used <= static_cast<int32_t>(vec.size()));

        char* data = reinterpret_cast<char *>(vec.data());
        char* start = mapped_region_begin + m_pos;
        char* end = start + total_size;
        std::copy(start, end, data);
        m_pos += total_size;
    };

    void setPosFile(int64_t pos);

    int64_t getPosFile();

    int32_t read_int();

    void write_int(const int32_t i);

    void write_byte(uint8_t i);

    uint8_t read_byte();

    void setPosEndFile();

    bool isEmpty();

private:
    template <typename T>
    std::int64_t write_to_dst(T val, int64_t dst) {
        int64_t total_size = sizeof(T);
        if (m_pos + total_size > m_size) {
            resize(std::max(2 * m_size, total_size));
        }
        char* data = reinterpret_cast<char *>(&val);
        std::copy(data, data + total_size, mapped_region_begin + dst);
        return dst + total_size;
    }

    void resize(int64_t new_size);
    void remap();

    const std::string path;
    char* mapped_region_begin;
    int64_t m_pos;
    int64_t m_size;
    int64_t m_capacity;
    bip::file_mapping file_mapping;
    bip::mapped_region mapped_region;
};

void createFile(const std::string &fn, std::int64_t bytes_num);
void writeToFile(const std::string &fn, std::uint64_t pos, std::int32_t val);
void resizeFile(const std::string &fn);