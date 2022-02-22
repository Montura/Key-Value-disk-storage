#pragma once

#include <iostream>
#include <memory>

#include "utils/utils.h"


namespace btree {
namespace file {
    void create_file(const std::string& path, const int64_t size) {
        std::ofstream ofs(path);
        if (!ofs.is_open())
            throw std::runtime_error("Can't create file for mapping, path = " + path);
        ofs.close();
        fs::resize_file(path, size);
    }
}
    using namespace utils;

    MappedFile::MappedFile(const std::string& path, const int64_t bytes_num) :
        m_pos(0),
        lru_cache(4096, 1000, path),
        path(path)
    {
        bool file_exists = fs::exists(path);
        if (!file_exists) {
            file::create_file(path, bytes_num);
            m_capacity = bytes_num;
        } else {
            m_capacity = static_cast<int64_t>(fs::file_size(path));
        }
    }
 
    MappedFile::~MappedFile() {
/**  _MSC_VER
 * 1. Faced with the same error as in: https://youtrack.jetbrains.com/issue/PROF-752
    - [WIN32 error] = 1224, The requested operation cannot be performed on a file with a user-mapped section open.

 * 2. See impl of std::filesystem::resize_file
    - https://github.com/microsoft/STL/blob/main/stl/src/filesystem.cpp#L671
       - Use WinAPI BOOL SetEndOfFile([in] HANDLE hFile) for Windows platform
 * 3. Doc https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-setendoffile
    - CreateFileMapping is called to create a file mapping object for hFile,
      UnmapViewOfFile must be called first to unmap all views and call CloseHandle to close
      the file mapping object before you can call SetEndOfFile.
 * 4. See impl of BOOST_MAPPED_REGION dtor:
    - https://github.com/steinwurf/boost/blob/master/boost/interprocess/mapped_region.hpp#L555
*/
        std::error_code error_code;
        fs::resize_file(path, m_capacity, error_code);
        if (error_code)
            std::cerr << "Can't resize file: " << path << std::endl;
    }

    template <typename T>
    void MappedFile::write_next_data(std::unique_ptr<MappedRegion>& region, T val, const int32_t total_size_in_bytes) {
        if constexpr(std::is_pointer_v<T>)
            m_pos = write_blob(region, val, total_size_in_bytes);
        else
            m_pos = write_next_primitive(region, val);
        m_capacity = std::max(m_pos, m_capacity);
    }

    template <typename ValueType>
    std::pair<ValueType, int32_t> MappedFile::read_next_data(const std::unique_ptr<MappedRegion>& region) {
        return region->template read_next_data<ValueType>();
    }

    template <typename T>
    int64_t MappedFile::write_next_primitive(const int64_t pos, const T val) {
        static_assert(std::is_arithmetic_v<T>);
        std::unique_lock lock(mutex);

        auto block4kb_ptr = lru_cache.on_new_pos(pos);
        const auto total_bytes_to_write = sizeof(T);
        const int64_t new_size = block4kb_ptr->current_absolute_pos() + total_bytes_to_write;
        if (new_size > m_capacity) {
            fs::resize_file(path, new_size);
        }

        auto bytes_to_write = total_bytes_to_write;
        auto curr_pos = pos + total_bytes_to_write;
        while (bytes_to_write > 0) {
            auto [bytes_was_written, remaining_bytes] = block4kb_ptr->write_next_primitive(val, bytes_to_write);
            if (remaining_bytes) {
                block4kb_ptr = lru_cache.on_new_pos(curr_pos + bytes_was_written);
                curr_pos %= block4kb_ptr->m_size;
            }
            bytes_to_write -= bytes_was_written;
        }

        m_capacity = std::max(new_size, m_capacity);
        return curr_pos;
    }

    template <typename T>
    std::pair<T, int64_t> MappedFile::read_next_primitive(const int64_t pos) {
        static_assert(std::is_arithmetic_v<T>);

        const auto block4kb_ptr = lru_cache.on_new_pos(pos);
        const int64_t total_bytes_to_read = sizeof(T);

        T value = block4kb_ptr->read_next_primitive<T>(pos, total_bytes_to_read);

        return std::make_pair(value, pos + total_bytes_to_read);
    }

    template <typename T>
    int64_t MappedFile::write_next_primitive(std::unique_ptr<MappedRegion>& region, const T val) {
        static_assert(std::is_arithmetic_v<T>);
        int64_t new_size = resize(region, sizeof(T));

        m_capacity = std::max(new_size, m_capacity);
        m_pos = region->write_next_primitive(val);
        return m_pos;
    }

    template <typename T>
    T MappedFile::read_next_primitive(const std::unique_ptr<MappedRegion>& region) {
        return region->template read_next_primitive<T>();
    }

    template <typename T>
    void MappedFile::write_node_vector(std::unique_ptr<MappedRegion>& region, const std::vector<T>& vec) {
        int64_t total_size_in_bytes = sizeof(T) * vec.size();
        write_blob(region, vec.data(), total_size_in_bytes);
    }

    template <typename T>
    void MappedFile::read_node_vector(const std::unique_ptr<MappedRegion>& region, std::vector<T>& vec) {
        int64_t total_size_in_bytes = sizeof(T) * vec.size();

        auto* data = cast_to_uint8_t_data(vec.data());
        auto [raw_data, len] = region->template read_next_data<const uint8_t*>();
        std::copy(raw_data, raw_data + len, data);
        m_pos += total_size_in_bytes;
    }

    template <typename T>
    int64_t MappedFile::write_blob(std::unique_ptr<MappedRegion>& region, T source_data, const int32_t total_size_in_bytes) {
        // write size
        int32_t len = total_size_in_bytes;
        int64_t region_pos = write_next_primitive(region, len);

        // write values
        int64_t new_size = resize(region, total_size_in_bytes);

        m_capacity = std::max(new_size, m_capacity);
        m_pos = region->write_blob(source_data, total_size_in_bytes);
        return m_pos;
    }

    int64_t MappedFile::resize(std::unique_ptr<MappedRegion>& region, int64_t total_size_in_bytes, bool shrink_to_fit) {
        int64_t current_pos = region->current_pos();
        int64_t new_size = current_pos + total_size_in_bytes;
        if (new_size > region->size()) {
            region = std::make_unique<MappedRegion>(current_pos, path);
            fs::resize_file(path, new_size);
            region->remap(bip::read_write, total_size_in_bytes);
        }
        return new_size;
    }

    std::unique_ptr<MappedRegion> MappedFile::get_mapped_region(int64_t pos) {
        m_pos = pos > 0 ? pos : 0;
        std::unique_ptr<MappedRegion> region(new MappedRegion(pos, path));
        if (m_pos > 0)
            region->remap(bip::read_write);
        return region;
    }

    int16_t MappedFile::read_int16(const std::unique_ptr<MappedRegion>& region) {
        return region->read_next_primitive<int16_t>();
    }

    int32_t MappedFile::read_int32(const std::unique_ptr<MappedRegion>& region) {
        return region->read_next_primitive<int32_t>();
    }

    int64_t MappedFile::get_pos() const {
        return m_pos;
    }

    uint8_t MappedFile::read_byte(const std::unique_ptr<MappedRegion>& region) {
        return region->read_next_primitive<uint8_t>();
    }

    void MappedFile::set_file_pos_to_end() {
        m_pos = m_capacity;
    }

    void MappedFile::shrink_to_fit() {
        m_capacity = m_pos;
        fs::resize_file(path, m_capacity);
    }

    bool MappedFile::is_empty() const {
        return m_capacity == 0;
    }
}