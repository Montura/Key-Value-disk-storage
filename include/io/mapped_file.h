#pragma once

#include <string>
#include <fstream>
#include <filesystem>

#include "utils/boost_include.h"
#include "utils/utils.h"

namespace fs = std::filesystem;

namespace btree {
    class MappedRegion {
        const std::string path;
        bip::offset_t file_pos;
        bip::mapped_region mapped_region;
        uint8_t* mapped_region_begin;
        bip::offset_t m_pos;
        
        uint8_t* read_only_address_for_offset(int64_t offset) {
            auto end_address = m_pos + offset;
            if (end_address > static_cast<int64_t>(mapped_region.get_size())) {
                remap(bip::read_only, file_pos, calc_new_size(end_address));
                end_address = offset;
            }
            uint8_t* address_begin = address_by_offset(m_pos);
            m_pos = end_address;
            return address_begin;
        }
    public:
        explicit MappedRegion(int64_t file_pos,  const std::string& path);
        uint8_t* address_by_offset(const int64_t offset) const;
        void remap(bip::mode_t mode = bip::read_write, bip::offset_t file_offset = 0, size_t size = 0);

        template <typename T>
        T read_next_primitive();

        template <typename ValueType>
        std::pair<ValueType, int32_t> read_next_data();

//        /** Warning: do not write vector size */
//        template <typename T>
//        void write_node_vector(const std::vector<T>& vec);

        int64_t calc_new_size(int64_t end_address) const {
            int64_t fileSize = fs::file_size(path);
            if (end_address <= fileSize) {
//                int64_t delta = end_address - m_pos;
                int64_t new_size = 128;
                if (m_pos + new_size > fileSize) {
                    return fileSize - m_pos;
                } else {
                    return new_size;
                }
            } else {
                throw std::runtime_error("Attempted to read from memory outside the mapped region");
            }
        }
    };

    class MappedFile {
//        using ValueType = utils::conditional_t<std::is_arithmetic_v<V>, const V, const uint8_t*>;

        int64_t m_pos;
        int64_t m_size;
        int64_t m_capacity;
        std::unique_ptr<MappedRegion> m_mapped_region;
    public:
        const std::string path;

        MappedFile(const std::string& fn, const int64_t bytes_num);

        ~MappedFile();

        template <typename ValueType>
        std::pair<ValueType, int32_t> read_next_data(MappedRegion* region);

        template <typename T>
        void write_next_primitive(const T val);

        template <typename T>
        T read_next_primitive(MappedRegion* region);

        template <typename T>
        void write_next_data(T val, const int32_t total_size_in_bytes);

        /** Warning: do not write vector size */
        template <typename T>
        void write_node_vector(const std::vector<T>& vec);

        /** Warning: do not read vector size */
        template <typename T>
        void read_node_vector(std::vector<T>& vec);

        int64_t get_pos() const;
        std::unique_ptr<MappedRegion> set_pos(int64_t pos);
        void set_file_pos_to_end();

        uint8_t read_byte(MappedRegion* region);
        int16_t read_int16(MappedRegion* region);
        int32_t read_int32(MappedRegion* region);

        void shrink_to_fit();
        bool is_empty() const;

    private:
        template <typename T>
        int64_t write_arithmetic(T val);

        template <typename T>
        int64_t write_blob(T source_data, const int32_t total_size_in_bytes);

        void resize(int64_t new_size, bool shrink_to_fit = false);

        constexpr int64_t scale_current_size() const {
            return static_cast<int64_t>(m_size * 1.1);
        }
    };
}

#include "mapped_file_impl.h"