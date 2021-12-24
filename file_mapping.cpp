#include "file_mapping.h"

MappedFile::MappedFile(const std::string &fn, std::int64_t bytes_num) : path(fn), m_pos(0) {
    std::filebuf fbuf;
    fbuf.open(path.data(), std::ios_base::out | std::ios_base::trunc);
    fbuf.pubseekoff(bytes_num, std::ios_base::beg);
    fbuf.sputc(0);
    fbuf.close();

    m_size = bytes_num;
    remap();
}

MappedFile::~MappedFile() {}

std::int32_t MappedFile::readInt(std::int64_t f_pos) {
    return *(mapped_region_begin + f_pos * sizeof(std::int32_t));
}

void MappedFile::writeInt(std::int32_t val) {
    if (2 * m_pos > m_size) {
        resize(m_size);
    }
    char* value_begin = reinterpret_cast<char *>(&val);
    int value_size = sizeof(std::int32_t);
    m_pos += value_size;
//        cout << "my m_pos: " << m_pos  << endl;
    std::copy(value_begin, value_begin + value_size, mapped_region_begin + m_pos);
//        *(mapped_region_begin + m_pos) = val;
}

void MappedFile::resize(std::int64_t bytes_num) {
    m_size = (m_size != 0) ? m_size * 2 : bytes_num;

    std::filebuf fbuf;
    fbuf.open(path.data(), std::ios_base::in | std::ios_base::out);
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
    mapped_region_begin = reinterpret_cast<char *>(mapped_region.get_address());
}


void createFile(const std::string &fn, std::int64_t bytes_num) {
    std::filebuf fbuf;
    fbuf.open(fn, std::ios_base::out | std::ios_base::trunc);

    fbuf.pubseekoff(bytes_num, std::ios_base::beg);
    fbuf.sputc(0);
    fbuf.close();
}

void writeToFile(const std::string &fn, std::uint64_t pos, std::int32_t val) {
    bip::file_mapping fm(fn.data(), bip::read_write);
    bip::mapped_region rg(fm, bip::read_write);

    char *p = reinterpret_cast<char *>(rg.get_address());
//    cout << "m_pos: " << m_pos * 4 << endl;
    *(p + pos * 4) = val;       // write value into position
}

void resizeFile(const std::string &fn) {
    constexpr auto offset = sizeof(std::uint64_t) * 6 - 1;

    std::filebuf fbuf;
    fbuf.open(fn, std::ios_base::in | std::ios_base::out);
    fbuf.pubseekoff(offset, std::ios_base::beg);
    fbuf.sputc(0);
    fbuf.close();
}
