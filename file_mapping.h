#include <string>
#include <fstream>
#include "boost_include.h"

struct MappedFile {
    MappedFile(const std::string& fn, int64_t bytes_num);
    ~MappedFile();

    template <typename T>
    T read_next();

    template <typename T>
    void write_next(T val);

    template <typename T>
    void write(T val, const int64_t pos);

//    void write_string(const std::string& s);

    void write_int_array(int* vec, int32_t used);
    void read_int_array(int* vec, int32_t len);

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
    std::int64_t write_to_dst(T val, std::int64_t dst);

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