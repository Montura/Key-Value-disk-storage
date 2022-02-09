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
            m_pos(0), m_mapped_region(new MappedRegion(0, path)), path(path)
    {
        bool file_exists = fs::exists(path);
        if (!file_exists) {
            file::create_file(path, bytes_num);
            m_size = m_capacity = bytes_num;
        } else {
            m_size = m_capacity = static_cast<int64_t>(fs::file_size(path));
        }
        if (m_size > 0)
            m_mapped_region->remap();
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
        m_mapped_region.reset(nullptr);
        std::error_code error_code;
        fs::resize_file(path, m_capacity, error_code);
        if (error_code)
            std::cerr << "Can't resize file: " << path << std::endl;
    }

    MappedRegion::MappedRegion(int64_t file_pos, const std::string& path)
        : path(path), file_pos(file_pos), mapped_region_begin(nullptr), m_pos(0) {}

    uint8_t* MappedRegion::address_by_offset(int64_t offset) const {
        return mapped_region_begin + offset;
    }

    void MappedRegion::remap(bip::mode_t mode, bip::offset_t file_offset, size_t size) {
        if (mode == bip::read_only) {
            file_pos += m_pos;
            m_pos = 0;
        } else {
            file_pos = file_offset;
        }
        auto file_mapping = bip::file_mapping(path.data(), mode);
        auto tmp_mapped_region = bip::mapped_region(file_mapping, mode, file_pos, size);
        mapped_region.swap(tmp_mapped_region);
        mapped_region_begin = cast_to_uint8_t_data(mapped_region.get_address());
    }

    template <typename T>
    void MappedFile::write_next_data(T val, const int32_t total_size_in_bytes) {
        if constexpr(std::is_pointer_v<T>)
            m_pos = write_blob(val, total_size_in_bytes);
        else
            m_pos = write_arithmetic(val);
        m_capacity = std::max(m_pos, m_capacity);
    }

    template <typename ValueType>
    std::pair<ValueType, int32_t> MappedRegion::read_next_data() {
        if constexpr(std::is_pointer_v<ValueType>) {
            auto len = read_next_primitive<int32_t>();
            auto being_address = read_only_address_for_offset(len);
            return std::make_pair(cast_to_const_uint8_t_data(being_address), len);
        } else {
            static_assert(std::is_arithmetic_v<ValueType>);
            return std::make_pair(read_next_primitive<ValueType>(), static_cast<int32_t>(sizeof(ValueType)));
        }
    }

    template <typename ValueType>
    std::pair<ValueType, int32_t> MappedFile::read_next_data(MappedRegion* region) {
        return region->template read_next_data<ValueType>();
    }

    template <typename T>
    void MappedFile::write_next_primitive(const T val) {
        static_assert(std::is_arithmetic_v<T>);
        m_pos = write_arithmetic(val);
        m_capacity = std::max(m_pos, m_capacity);
    }

    template <typename T>
    T MappedRegion::read_next_primitive() {
        static_assert(std::is_arithmetic_v<T>);
        auto being_address = read_only_address_for_offset(sizeof(T));
        return *(reinterpret_cast<T*>(being_address));
    }

    template <typename T>
    T MappedFile::read_next_primitive(MappedRegion* region) {
        return region->template read_next_primitive<T>();
    }

    template <typename T>
    void MappedFile::write_node_vector(const std::vector<T>& vec) {
        int64_t total_size_in_bytes = sizeof(T) * vec.size();
        if (m_pos + total_size_in_bytes > m_size)
            resize(m_pos + total_size_in_bytes);

        auto* data = cast_to_const_uint8_t_data(vec.data());
        std::copy(data, data + total_size_in_bytes, m_mapped_region->address_by_offset(m_pos));
        m_pos += total_size_in_bytes;
        m_capacity = std::max(m_pos, m_capacity);
    }

    template <typename T>
    void MappedFile::read_node_vector(std::vector<T>& vec) {
        int64_t total_size = sizeof(T) * vec.size();

        auto* data = cast_to_uint8_t_data(vec.data());
        auto* start = m_mapped_region->address_by_offset(m_pos);
        auto* end = start + total_size;
        std::copy(start, end, data);
        m_pos += total_size;
    }

    template <typename T>
    int64_t MappedFile::write_arithmetic(T val) {
        static_assert(std::is_arithmetic_v<T>);
        int64_t total_size_in_bytes = sizeof(T);
        if (m_pos + total_size_in_bytes > m_size)
            resize(m_pos + total_size_in_bytes);

        auto* data = cast_to_const_uint8_t_data(&val);
        std::copy(data, data + total_size_in_bytes, m_mapped_region->address_by_offset(m_pos));
        return m_pos + total_size_in_bytes;
    }

    template <typename T>
    int64_t MappedFile::write_blob(T source_data, const int32_t total_size_in_bytes) {
        // write size
        int32_t len = total_size_in_bytes;
        m_pos = write_arithmetic(len);

        // write values
        int64_t total_bytes_size = total_size_in_bytes;
        if (m_pos + total_bytes_size > m_size)
            resize(m_pos + total_bytes_size);

        auto* data = cast_to_const_uint8_t_data(source_data);
        std::copy(data, data + total_bytes_size, m_mapped_region->address_by_offset(m_pos));
        return m_pos + total_bytes_size;
    }

    void MappedFile::resize(int64_t new_size, bool shrink_to_fit) {
        m_size = shrink_to_fit ? new_size : std::max(scale_current_size(), new_size);
        m_mapped_region = std::make_unique<MappedRegion>(m_pos, path);
        fs::resize_file(path, m_size);
        m_mapped_region->remap();
    }

    std::unique_ptr<MappedRegion> MappedFile::set_pos(int64_t pos) {
        m_pos = pos > 0 ? pos : 0;
        std::unique_ptr<MappedRegion> region(new MappedRegion(pos, path));
        region->remap(bip::read_write, m_pos);
        return region;
    }

    int16_t MappedFile::read_int16(MappedRegion* region) {
        return region->read_next_primitive<int16_t>();
    }

    int32_t MappedFile::read_int32(MappedRegion* region) {
        return region->read_next_primitive<int32_t>();
    }

    int64_t MappedFile::get_pos() const {
        return m_pos;
    }

    uint8_t MappedFile::read_byte(MappedRegion* region) {
        return region->read_next_primitive<uint8_t>();
    }

    void MappedFile::set_file_pos_to_end() {
        m_pos = m_capacity;
    }

    void MappedFile::shrink_to_fit() {
        m_capacity = m_size = m_pos;
        resize(m_size, true);
    }

    bool MappedFile::is_empty() const {
        return m_size == 0;
    }
}