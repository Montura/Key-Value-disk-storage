#pragma once

#include <string>
#include <fstream>

#include "boost_include.h"

struct MappedFile {
    MappedFile(const std::string& fn, int64_t bytes_num);
    ~MappedFile();

    // todo: hide form public?
    template <typename T>
    T read_next();

    template <typename T>
    T read_container();

    template <typename T>
    void write_next(T val);

    /** Warning: do not write vector size */
    template <typename T>
    void write_node_vector(const std::vector<T>& vec);

    /** Warning: do not read vector size */
    template <typename T>
    void read_node_vector(std::vector<T>& vec);

    void set_pos(int64_t pos);
    int64_t get_pos();

    int16_t read_int16();
    int32_t read_int32();
    int64_t read_int64();
    uint8_t read_byte();

    void set_file_pos_to_end();
    void shrink_to_fit();
    bool isEmpty();

private:
    template <typename T>
    int64_t write_arithmetic(T val);

    template <typename T>
    int64_t write_container(T val);

    void resize(int64_t new_size);
    void remap();

    const std::string path;
    uint8_t* mapped_region_begin;
    int64_t m_pos;
    int64_t m_size;
    int64_t m_capacity;
    bip::file_mapping file_mapping;
    bip::mapped_region mapped_region;
};

#include "file_mapping_impl.h"