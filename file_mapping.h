#pragma once

#include <string>
#include <fstream>

#include "boost_include.h"

struct MappedFile {
    const std::string path;

    MappedFile(const std::string& fn, int64_t bytes_num);
    ~MappedFile();

    template <typename T>
    T read_next();

    template <typename T>
    T read_string();

    template <typename T>
    void write_next(T val);

//    template <typename T>
//    void write(T val, int64_t f_pos);

    template <typename T>
    void read_vector(std::vector<T>& vec);

    int64_t get_pos();
    int32_t read_int();
    uint8_t read_byte();

    void set_pos(int64_t pos);
    void set_file_pos_to_end();

    bool isEmpty();

private:
    template <typename T>
    void write_vector(const std::vector<T>& vec);

    template <typename T>
    int64_t write_arithmetic_to_dst(T val, int64_t dst);

    template <typename T>
    int64_t write_string_to_dst(T val, int64_t dst);

    void resize(int64_t new_size);
    void remap();

    uint8_t* mapped_region_begin;
    int64_t m_pos;
    int64_t m_size;
    int64_t m_capacity;
    bip::file_mapping file_mapping;
    bip::mapped_region mapped_region;
};

#include "file_mapping_impl.h"