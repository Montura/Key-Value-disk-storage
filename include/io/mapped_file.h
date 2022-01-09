#pragma once

#include <string>
#include <fstream>

#include "utils/boost_include.h"

namespace btree {
    struct MappedFile {

        class MappedRegion {
            bip::mapped_region mapped_region;
            uint8_t* mapped_region_begin;
        public:
            explicit MappedRegion();
            uint8_t* address_by_offset(const int64_t offset);
            void remap(const std::string& path);
        };

        MappedFile(const std::string& fn, int64_t bytes_num);

        ~MappedFile();

        template <typename ValueType>
        std::pair<ValueType, int32_t> read_next_data();

        template <typename T>
        void write_next_primitive(const T val);

        template <typename T>
        T read_next_primitive();

        template <typename T>
        void write_next_data(const T& val, const int32_t total_size_in_bytes);

        /** Warning: do not write vector size */
        template <typename T>
        void write_node_vector(const std::vector<T>& vec);

        /** Warning: do not read vector size */
        template <typename T>
        void read_node_vector(std::vector<T>& vec);

        int64_t get_pos() const;
        void set_pos(int64_t pos);
        void set_file_pos_to_end();

        uint8_t read_byte();
        int16_t read_int16();
        int32_t read_int32();
        int64_t read_int64();

        void shrink_to_fit();
        bool isEmpty() const;

        const std::string path;
    private:
        template <typename T>
        int64_t write_arithmetic(T val);

        template <typename T>
        int64_t write_blob(T source_data, const int32_t total_size_in_bytes);

        void resize(int64_t new_size);

        int64_t m_pos;
        int64_t m_size;
        int64_t m_capacity;
        MappedRegion* m_mapped_region;
    };
}
#include "mapped_file_impl.h"