#pragma once

#include <string>
#include <fstream>
#include <filesystem>

#include "mapped_region.h"
#include "utils/boost_include.h"
#include "utils/utils.h"

namespace fs = std::filesystem;

namespace btree {
    class MappedFile {
        int64_t m_pos;
        int64_t m_capacity;
    public:
        const std::string path;

        MappedFile(const std::string& fn, const int64_t bytes_num);

        ~MappedFile();

        template <typename ValueType>
        std::pair<ValueType, int32_t> read_next_data(MappedRegion* region);

        template <typename T>
        int64_t write_next_primitive(std::unique_ptr<MappedRegion>& region, const T val);

        template <typename T>
        T read_next_primitive(MappedRegion* region);

        template <typename T>
        void write_next_data(std::unique_ptr<MappedRegion>& region, T val, const int32_t total_size_in_bytes);

        /** Warning: do not write vector size */
        template <typename T>
        void write_node_vector(std::unique_ptr<MappedRegion>& region, const std::vector<T>& vec);

        /** Warning: do not read vector size */
        template <typename T>
        void read_node_vector(const std::unique_ptr<MappedRegion>& region, std::vector<T>& vec);

        int64_t get_pos() const;
        std::unique_ptr<MappedRegion> get_mapped_region(int64_t pos);
        void set_file_pos_to_end();

        uint8_t read_byte(MappedRegion* region);
        int16_t read_int16(MappedRegion* region);
        int32_t read_int32(MappedRegion* region);

        void shrink_to_fit();
        bool is_empty() const;

    private:
        template <typename T>
        int64_t write_blob(std::unique_ptr<MappedRegion>& region, T source_data, const int32_t total_size_in_bytes);

        int64_t resize(std::unique_ptr<MappedRegion>& region, int64_t total_size_in_bytes, bool shrink_to_fit = false);
    };
}

#include "mapped_file_impl.h"