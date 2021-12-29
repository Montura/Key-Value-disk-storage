#include <string>
#include <fstream>
#include "boost_include.h"

template <typename T>
struct is_string {
    static constexpr bool value = false;
};

template <typename CharT, typename Traits, typename Alloc>
struct is_string <std::basic_string<CharT, Traits, Alloc>> {
    static constexpr bool value = true;
};

template <typename T>
static constexpr bool is_string_v = is_string<T>::value;

template <typename T>
using to_string = std::basic_string<T> (*)(int i);

template <typename T>
struct is_vector {
    static constexpr bool value = false;
};

template <typename T, typename Alloc>
struct is_vector <std::vector<T, Alloc>> {
    static constexpr bool value = true;
};

template <typename T>
static constexpr bool is_vector_v = is_vector<T>::value;


struct MappedFile {
    MappedFile(const std::string& fn, int64_t bytes_num);
    ~MappedFile();

    // todo: hide form public?
    template <typename T>
    T read_next() {
//        if constexpr(std::is_arithmetic_v<T>) {
        static_assert(std::is_arithmetic_v<T>);
        char *value_begin = mapped_region_begin + m_pos;
        m_pos += sizeof(T);
        return *(reinterpret_cast<T *>(value_begin));
//        } else {
//            return read_string<T>();
//        }
    }
//
//    template <typename T>
//    T read_string() {
//        int32_t elem_count = read_next<typename T::size_type>();
//        T str(elem_count, '\0');
//
//        uint32_t total_size = sizeof(typename T::value_type) * elem_count;
//
//        char* data = reinterpret_cast<char *>(str.data());
//        char* start = mapped_region_begin + m_pos;
//        char* end = start + total_size;
//        std::copy(start, end, data);
//        m_pos += total_size;
//        return str;
//    }

    template <typename T>
    void write_next(T val) {
        if constexpr(std::is_arithmetic_v<T>) {
            static_assert(std::is_arithmetic_v<T>);
            m_pos = write_arithmetic(val);
        } else {
            static_assert(is_string_v<T> || is_vector_v<T>);
            m_pos = write_container(val);
        }
        m_capacity = std::max(m_pos, m_capacity);
    }

    template <typename T>
    void write_node_vector(const std::vector<T>& vec) {
        int64_t total_size = static_cast<int64_t>(sizeof(T)) * vec.size();
        if (m_pos + total_size > m_size) {
            resize(std::max(2 * m_size, m_pos + total_size));
        }
        const char* data = reinterpret_cast<const char *>(vec.data());
        std::copy(data, data + total_size, mapped_region_begin + m_pos);
        m_pos += total_size;
        m_capacity = std::max(m_pos, m_capacity);
    }

    /** Warning: do not read vector size */
    template <typename T>
    void read_node_vector(std::vector<T>& vec) {
        uint32_t total_size = sizeof(T) * vec.size();

        char* data = reinterpret_cast<char *>(vec.data());
        char* start = mapped_region_begin + m_pos;
        char* end = start + total_size;
        std::copy(start, end, data);
        m_pos += total_size;
    };

    void set_pos(int64_t pos);

    int64_t get_pos();

    int32_t read_int();

    uint8_t read_byte();

    void set_file_pos_to_end();

    bool isEmpty();

private:
    template <typename T>
    std::int64_t write_arithmetic(T val) {
        int64_t total_size = sizeof(T);
        if (m_pos + total_size > m_size) {
            resize(std::max(2 * m_size, m_pos + total_size));
        }
        char* data = reinterpret_cast<char *>(&val);
        std::copy(data, data + total_size, mapped_region_begin + m_pos);
        return m_pos + total_size;
    }

    template <typename T>
    std::int64_t write_container(T val) {
        // write size
        size_t elem_count = val.size();
        m_pos = write_arithmetic(elem_count);

        // write values
        int64_t total_bytes_size = sizeof(typename T::value_type) * elem_count;
        if (m_pos + total_bytes_size > m_size) {
            resize(std::max(2 * m_size, total_bytes_size));
        }

        char* data = reinterpret_cast<char *>(val.data());
        std::copy(data, data + total_bytes_size, mapped_region_begin + m_pos);
        return m_pos + total_bytes_size;
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