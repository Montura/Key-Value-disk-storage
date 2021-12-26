#include <filesystem>

#include "file_mapping.h"

namespace fs = std::filesystem;

MappedFile::MappedFile(const std::string &fn, int64_t bytes_num) : path(fn), m_pos(0) {
    bool file_exists = fs::exists(fn);
    if (!file_exists) {
        std::filebuf fbuf;
        fbuf.open(path, std::ios_base::out | std::ios_base::trunc);
        fbuf.pubseekoff(bytes_num, std::ios_base::beg);
        fbuf.sputc(0);
        fbuf.close();
        m_size = m_capacity = bytes_num;
    } else {
        m_size = m_capacity = static_cast<int64_t>(fs::file_size(fn));
    }
    remap();
}

MappedFile::~MappedFile() {
#ifndef _MSC_VER_
    fs::resize_file(path,m_capacity);
#endif
}


void MappedFile::resize(int64_t new_size) {
    m_size = new_size;

    std::filebuf fbuf;
    fbuf.open(path, std::ios_base::in | std::ios_base::out);
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

void MappedFile::write_int_array(int* vec, const std::int32_t used) {
    int64_t total_size = static_cast<int64_t>(sizeof(int32_t)) * used;
    if (m_pos + total_size > m_size) {
        resize(std::max(2 * m_size, total_size));
    }
    const char* data = reinterpret_cast<const char *>(vec);
    std::copy(data, data + total_size, mapped_region_begin + m_pos);
    m_pos += total_size;
    m_capacity = std::max(m_pos, m_capacity);
}

void MappedFile::read_int_array(int* vec, int32_t used){
    uint32_t total_size = sizeof(int32_t) * used;
//    assert(used <= static_cast<int32_t>(vec.size()));

    char* data = reinterpret_cast<char *>(vec);
    char* start = mapped_region_begin + m_pos;
    char* end = start + total_size;
    std::copy(start, end, data);
    m_pos += total_size;
}

void MappedFile::set_pos(int64_t pos) {
    m_pos = pos > 0 ? pos : 0;
}

int32_t MappedFile::read_int() {
    return read_next<int32_t>();
}

int64_t MappedFile::get_pos() {
    return m_pos;
}

void MappedFile::write_int(const int32_t val) {
    write_next(val);
}

void MappedFile::write_byte(uint8_t val) {
    write_next(val);
}

uint8_t MappedFile::read_byte() {
    return read_next<uint8_t>();
}

void MappedFile::set_file_pos_to_end() {
    m_pos = m_capacity;
}

bool MappedFile::isEmpty() {
    return m_size == 0;
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


constexpr int ITERATIONS = 1000;

template <typename T>
void test_arithmetic_types(T val_to_add) {
    std::string fmap = std::string("../file_mapping_text") + std::string(".txt");
    if (fs::exists(fmap)) {
        fs::remove(fmap);
    }
    MappedFile file(fmap, 32);

    // write
    for (int i = 0; i < ITERATIONS; ++i) {
        T tmp = val_to_add + i;
        file.write_next(tmp);
    }

    // read
    file.set_pos(0);
    for (int i = 0; i < ITERATIONS; ++i) {
        T tmp = file.read_next<T>();
        assert(tmp == val_to_add + i);
    }
}


template <typename T, typename V>
void test_basic_strings(const V* val, to_string<V> converter) {
    std::string fmap = std::string("../file_mapping_text") + std::string(".txt");
    if (fs::exists(fmap)) {
        fs::remove(fmap);
    }
    MappedFile file(fmap, 32);
    auto value = T(val);

    // write
    for (auto i = 0; i < ITERATIONS; ++i) {
        T tmp = value + converter(i);
        file.write_next(tmp);
    }

    // read
    file.set_pos(0);
    for (int i = 0; i < ITERATIONS; ++i) {
        T tmp = file.read_next<T>();
        assert(tmp == value + converter(i));
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
            auto anInt = file.read_int();
            if (i % 1000 == 0) {
                assert(anInt == -1);
            } else {
                assert(i == anInt);
            }
        }
    }
}

void test_array() {
    std::string fmap = std::string("../file_mapping_array") + std::string(".txt");
    if (fs::exists(fmap)) {
        fs::remove(fmap);
    }

    int n = 1000000;
//    std::vector<int> out(n, 1);
//    std::vector<int> in(n, 0);
    {
        MappedFile file(fmap, 32);
//        file.write_int_array(out, n);
    }

    {
        MappedFile file(fmap, 32);
//        file.read_int_array(in, n);
    }
//    assert(in == out);
}

#include <codecvt>
//namespace {
//    BOOST_AUTO_TEST_CASE(file_mapping_test) {
int main() {
    test_arithmetic_types<int32_t>(1);
    test_arithmetic_types<uint32_t>(1);
    test_arithmetic_types<int64_t>(1);
    test_arithmetic_types<uint64_t>(1);
    test_arithmetic_types<float>(1);
    test_arithmetic_types<double>(1);


    to_string<char> conv_to_str = std::to_string;
    to_string<wchar_t> conv_to_wstr = std::to_wstring;

    std::string strs[] = { "", "a", "aba", "abacaba", "abba", "abacabacababa" };
    std::wstring wstrs[] = { L"", L"a", L"aba", L"abacaba", L"abba", L"abacabacababa" };

    for (auto& str : strs) {
        test_basic_strings<std::string>(str.c_str(), conv_to_str);
    }

    for (auto& wstr : wstrs) {
        test_basic_strings<std::wstring>(wstr.c_str(), conv_to_wstr);
    }

//    test_basic_strings<std::string>("2ul", t);
//    test_basic_strings<std::wstring>(L"2ul", t1);

//    test_modify_and_save();
//    test_array();

    std::string msg = "File mapping test";
//        BOOST_REQUIRE_MESSAGE(success, msg);
    return 0;
}
//    }
//}
//#endif