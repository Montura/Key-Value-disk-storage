#include <filesystem>

#include "file_mapping.h"

namespace fs = std::filesystem;

MappedFile::MappedFile(const std::string &fn, std::int64_t bytes_num) : path(fn), m_pos(0) {
    bool file_exists = fs::exists(fn);
    if (!file_exists) {
        std::filebuf fbuf;
        fbuf.open(path.data(), std::ios_base::out | std::ios_base::trunc);
        fbuf.pubseekoff(bytes_num, std::ios_base::beg);
        fbuf.sputc(0);
        fbuf.close();
        m_size = bytes_num;
    } else {
        m_size = static_cast<int64_t>(fs::file_size(fn));
    }
    remap();
}

MappedFile::~MappedFile() {}

template <typename T>
T MappedFile::read_next(std::int64_t f_pos) {
    static_assert(std::is_arithmetic_v<T>);
    char *value_begin = mapped_region_begin + (f_pos) * sizeof(T);
    return *(reinterpret_cast<T*>(value_begin));
}

template <typename T>
void MappedFile::write(T val, std::int64_t f_pos) {
    static_assert(std::is_arithmetic_v<T>);
    m_pos = write_to_dst(val, f_pos * sizeof(T));
}

template <typename T>
void MappedFile::write_next(T val) {
    static_assert(std::is_arithmetic_v<T>);
    m_pos = write_to_dst(val, m_pos);
}

template <typename T>
std::int64_t  MappedFile::write_to_dst(T val, std::int64_t dst) {
    if (2 * dst > m_size) {
        resize(m_size);
    }
    char* value_begin = reinterpret_cast<char *>(&val);
    int value_size = sizeof(T);
    std::copy(value_begin, value_begin + value_size, mapped_region_begin + dst);
    return dst + value_size;
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

//#ifdef UNIT_TESTS
//#if USE_BOOST_PREBUILT_STATIC_LIBRARY
//#include <boost/test/unit_test.hpp>
//#else
//#include <boost/test/included/unit_test.hpp>
//#endif
//
//#include <boost/test/data/test_case.hpp>
//#include <boost/range/iterator_range.hpp>


void test_create_and_write() {
    std::string fmap = std::string("../file_mapping_text") + std::string(".txt");
    if (fs::exists(fmap)) {
        fs::remove(fmap);
    }

    MappedFile file(fmap, 32);
    for (int i = 0; i < 1000000; ++i) {
        file.write_next(i);
    }
}

void test_read() {
    std::string fmap = std::string("../file_mapping_text") + std::string(".txt");
    MappedFile file(fmap, 32);
    for (int i = 0; i < 1000000; ++i) {
        auto anInt = file.read_next<int>(i);
        assert(i == anInt);
    }
}

void test_modify_and_save() {
    std::string fmap = std::string("../file_mapping_text") + std::string(".txt");
    {
        MappedFile file(fmap, 32);
        for (int i = 0; i < 1000000; i += 1000) {
            file.write(-1, i);
        }
    }
    {
        MappedFile file(fmap, 32);
        for (int i = 0; i < 1000000; ++i) {
            auto anInt = file.read_next<int>(i);
            if (i % 1000 == 0) {
                assert(anInt == -1);
            } else {
                assert(i == anInt);
            }
        }
    }
}

//namespace {
//    BOOST_AUTO_TEST_CASE(file_mapping_test) {
int main() {
//        std::string fn = std::string("../save") + std::strin g(".txt");
//    test_create_and_write();
    test_modify_and_save();

    std::string msg = "File mapping test";
//        BOOST_REQUIRE_MESSAGE(success, msg);
}
//    }
//}
//#endif