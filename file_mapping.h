#include <string>
#include <fstream>
#include "boost_include.h"

struct MappedFile {
    MappedFile(const std::string& fn, std::int64_t bytes_num);
    ~MappedFile();

    std::int32_t readInt(std::int64_t pos);
    void writeInt(std::int32_t val);

private:
    void resize(std::int64_t bytes_num);
    void remap();

    std::string_view path;
    char* mapped_region_begin;
    int64_t m_pos;
    int64_t m_size;
    bip::file_mapping file_mapping;
    bip::mapped_region mapped_region;
};

void createFile(const std::string &fn, std::int64_t bytes_num);
void writeToFile(const std::string &fn, std::uint64_t pos, std::int32_t val);
void resizeFile(const std::string &fn);