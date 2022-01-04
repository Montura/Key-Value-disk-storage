#pragma once

#include <filesystem>

#include "utils/utils.h"

namespace fs = std::filesystem;

namespace btree {
    MappedFile::MappedFile(const std::string& fn, int64_t bytes_num) : path(fn), m_pos(0) {
        bool file_exists = fs::exists(fn);
        if (!file_exists) {
            std::filebuf fbuf;
            auto* pFilebuf = fbuf.open(path, std::ios_base::out | std::ios_base::trunc);
            if (!pFilebuf)
                throw std::runtime_error("Wrong path is provided for mapped file, path = " + path);
            fbuf.pubseekoff(bytes_num, std::ios_base::beg);
            fbuf.sputc(0);
            fbuf.close();
            m_size = m_capacity = bytes_num;
        } else {
            m_size = m_capacity = static_cast<int64_t>(fs::file_size(fn));
        }
        if (m_size > 0)
            remap();
    }

    MappedFile::~MappedFile() {
#ifdef _MSC_VER
        //    std::cout << "RESIZE to " << m_capacity << std::endl;
            bip::file_mapping new_mapping;
            bip::mapped_region new_region;
            file_mapping.swap(new_mapping);
            mapped_region.swap(new_region);
            try {
                fs::resize_file(path, m_capacity);
            }
            catch(std::filesystem::filesystem_error const& ex) {
                std::cout
                        << "what():  " << ex.what() << '\n'
                        << "path1(): " << ex.path1() << '\n'
                        << "path2(): " << ex.path2() << '\n'
                        << "code().value():    " << ex.code().value() << '\n'
                        << "code().message():  " << ex.code().message() << '\n'
                        << "code().category(): " << ex.code().category().name() << '\n';
            }
#else
        fs::resize_file(path, m_capacity);
#endif
    }

    template <typename T>
    void MappedFile::write_next_data(const T& val, const int32_t total_size_in_bytes) {
        if constexpr(std::is_pointer_v<T>)
            m_pos = write_blob(val, total_size_in_bytes);
        else
            m_pos = write_arithmetic(val);
        m_capacity = std::max(m_pos, m_capacity);
    }

    template <typename EntryT>
    std::pair<typename EntryT::ValueType, int32_t> MappedFile::read_next_data() {
        typedef typename EntryT::ValueType ValueType;

        if constexpr(std::is_pointer_v<ValueType>) {
            auto len = read_next_primitive<int32_t>();
            auto* value_begin = mapped_region_begin + m_pos;
            m_pos += len;
            return std::make_pair(cast_to_const_uint8_t_data(value_begin), len);
        } else {
            static_assert(std::is_arithmetic_v<ValueType>);
            return std::make_pair(read_next_primitive<ValueType>(), sizeof(ValueType));
        }
    }

    template <typename T>
    void MappedFile::write_next_primitive(const T val) {
        static_assert(std::is_arithmetic_v<T>);
        m_pos = write_arithmetic(val);
        m_capacity = std::max(m_pos, m_capacity);
    }

    template <typename T>
    T MappedFile::read_next_primitive() {
        static_assert(std::is_arithmetic_v<T>);
        auto* value_begin = mapped_region_begin + m_pos;
        m_pos += sizeof(T);
        return *(reinterpret_cast<T*>(value_begin));
    }

    template <typename T>
    void MappedFile::write_node_vector(const std::vector<T>& vec) {
        int64_t total_size = sizeof(T) * vec.size();
        if (m_pos + total_size > m_size)
            resize(std::max(2 * m_size, m_pos + total_size));

        auto* data = cast_to_const_uint8_t_data(vec.data());
        std::copy(data, data + total_size, mapped_region_begin + m_pos);
        m_pos += total_size;
        m_capacity = std::max(m_pos, m_capacity);
    }

    template <typename T>
    void MappedFile::read_node_vector(std::vector<T>& vec) {
        int64_t total_size = sizeof(T) * vec.size();

        auto* data = cast_to_uint8_t_data(vec.data());
        auto* start = mapped_region_begin + m_pos;
        auto* end = start + total_size;
        std::copy(start, end, data);
        m_pos += total_size;
    }

    template <typename T>
    int64_t MappedFile::write_arithmetic(T val) {
        static_assert(std::is_arithmetic_v<T>);
        int64_t total_size = sizeof(T);
        if (m_pos + total_size > m_size)
            resize(std::max(2 * m_size, m_pos + total_size));

        auto* data = cast_to_const_uint8_t_data(&val);
        std::copy(data, data + total_size, mapped_region_begin + m_pos);
        return m_pos + total_size;
    }

    template <typename T>
    int64_t MappedFile::write_blob(T source_data, const int32_t total_size_in_bytes) {
        // write size
        int32_t len = total_size_in_bytes;
        m_pos = write_arithmetic(len);

        // write values
        int64_t total_bytes_size = total_size_in_bytes;
        if (m_pos + total_bytes_size > m_size)
            resize(std::max(2 * m_size, total_bytes_size));

        auto* data = cast_to_const_uint8_t_data(source_data);
        std::copy(data, data + total_bytes_size, mapped_region_begin + m_pos);
        return m_pos + total_bytes_size;
    }

    void MappedFile::resize(int64_t new_size) {
        m_size = new_size;

        std::filebuf fbuf;
        fbuf.open(path, std::ios_base::in | std::ios_base::out);
        fbuf.pubseekoff(m_size, std::ios_base::beg);
        fbuf.sputc(0);
        fbuf.close();

        remap();
    }

    void MappedFile::remap() {
        auto new_mapping = bip::file_mapping(path.data(), bip::read_write);
        auto new_region = bip::mapped_region(new_mapping, bip::read_write);
        file_mapping.swap(new_mapping);
        mapped_region.swap(new_region);
        mapped_region_begin = cast_to_uint8_t_data(mapped_region.get_address());
    }

    void MappedFile::set_pos(int64_t pos) {
        m_pos = pos > 0 ? pos : 0;
    }

    int16_t MappedFile::read_int16() {
        return read_next_primitive<int16_t>();
    }

    int32_t MappedFile::read_int32() {
        return read_next_primitive<int32_t>();
    }

    int64_t MappedFile::read_int64() {
        return read_next_primitive<int64_t>();
    }

    int64_t MappedFile::get_pos() const {
        return m_pos;
    }

    uint8_t MappedFile::read_byte() {
        return read_next_primitive<uint8_t>();
    }

    void MappedFile::set_file_pos_to_end() {
        m_pos = m_capacity;
    }

    void MappedFile::shrink_to_fit() {
        m_capacity = m_size = m_pos;
        resize(m_size);
        remap();
    }

    bool MappedFile::isEmpty() const {
        return m_size == 0;
    }
}