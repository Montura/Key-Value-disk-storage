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

template <typename T>
T MappedFile::read(std::int64_t f_pos) {
    static_assert(std::is_arithmetic_v<T>);
    char *value_begin = mapped_region_begin + (f_pos) * sizeof(T);
    return *(reinterpret_cast<T*>(value_begin));
}

template <typename T>
void MappedFile::write(T val) {
    static_assert(std::is_arithmetic_v<T>);
    if (2 * m_pos > m_size) {
        resize(m_size);
    }
    char* value_begin = reinterpret_cast<char *>(&val);
    int value_size = sizeof(T);
//        cout << "my m_pos: " << m_pos  << endl;
    std::copy(value_begin, value_begin + value_size, mapped_region_begin + m_pos);
    m_pos += value_size;
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


//namespace {
//    BOOST_AUTO_TEST_CASE(file_mapping_test) {
int main() {
//        std::string fn = std::string("../save") + std::strin g(".txt");
    std::string fmap = std::string("../file_mapping_text") + std::string(".txt");

    MappedFile file(fmap, 32);
    for (int i = 0; i < 1000000; ++i) {
        file.write(i);
    }

        bool success = true;
        for (int i = 0; i < 1000000; ++i) {
            int32_t anInt = file.read<int>(i);
            assert(i == anInt);
//            success &= b;
        }

    std::string msg = "File mapping test";
//        BOOST_REQUIRE_MESSAGE(success, msg);
}
//    }
//}
//#endif