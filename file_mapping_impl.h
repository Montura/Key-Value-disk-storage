#include <filesystem>

#include "utils.h"

namespace fs = std::filesystem;

MappedFile::MappedFile(const std::string &fn, int64_t bytes_num) : path(fn), m_pos(0) {
    bool file_exists = fs::exists(fn);
    if (!file_exists) {
        std::filebuf fbuf;
        auto *pFilebuf = fbuf.open(path, std::ios_base::out | std::ios_base::trunc);
        if (!pFilebuf)
            throw std::runtime_error("Wrong path is provided for mapped file, path = " + path);
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

template <typename T>
T MappedFile::read_next() {
    if constexpr(std::is_arithmetic_v<T>) {
        static_assert(std::is_arithmetic_v<T>);
        auto *value_begin = mapped_region_begin + m_pos;
        m_pos += sizeof(T);
        return *(reinterpret_cast<T *>(value_begin));
    } else {
        static_assert(is_string_v<T> || is_vector_v<T>);
        return read_container<T>();
    }
}

template <typename T>
T MappedFile::read_container() {
    int64_t elem_count = read_next<typename T::size_type>();
    T str(elem_count, '\0');

    int64_t total_size = sizeof(typename T::value_type) * elem_count;

    auto* data = cast_to_uint8_t_data(str.data());
    auto* start = mapped_region_begin + m_pos;
    auto* end = start + total_size;
    std::copy(start, end, data);
    m_pos += total_size;
    return str;
}

template <typename T>
void MappedFile::write_next(T val) {
    if constexpr(std::is_arithmetic_v<T>)
        m_pos = write_arithmetic_to_dst(val, m_pos);
    else
        m_pos = write_container_to_dst(val, m_pos);
    m_capacity = std::max(m_pos, m_capacity);
}

template <typename T>
void MappedFile::write_node_vector(const std::vector<T>& vec) {
    int64_t total_size = sizeof(T) * vec.size();
    if (m_pos + total_size > m_size)
        resize(std::max(2 * m_size, total_size));

    auto* data = cast_to_const_uint8_t_data(vec.data());
    std::copy(data, data + total_size, mapped_region_begin + m_pos);
    m_pos += total_size;
    m_capacity = std::max(m_pos, m_capacity);
}

template <typename T>
void MappedFile::read_node_vector(std::vector<T>& vec) {
    int64_t total_size = sizeof(T) * vec.size();

    auto* data = cast_to_uint8_t_data(vec.data());
    auto* start = mapped_region_begin + m_pos;
    auto* end = start + total_size;
    std::copy(start, end, data);
    m_pos += total_size;
}

template <typename T>
std::int64_t MappedFile::write_arithmetic_to_dst(T val, int64_t dst) {
    int64_t total_size = sizeof(T);
    if (m_pos + total_size > m_size)
        resize(std::max(2 * m_size, total_size));

    auto* data = cast_to_const_uint8_t_data(&val);
    std::copy(data, data + total_size, mapped_region_begin + dst);
    return dst + total_size;
}

template <typename T>
std::int64_t MappedFile::write_container_to_dst(T val, int64_t dst) {
    static_assert(is_string_v<T> || is_vector_v<T>);

    // write size
    int64_t elem_count = val.size();
    dst = write_arithmetic_to_dst(elem_count, dst);

    // write values
    int64_t total_bytes_size = sizeof(typename T::value_type) * elem_count;
    if (m_pos + total_bytes_size > m_size)
        resize(std::max(2 * m_size, total_bytes_size));

    auto* data = cast_to_uint8_t_data(val.data());
    std::copy(data, data + total_bytes_size, mapped_region_begin + dst);
    return dst + total_bytes_size;
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
    mapped_region_begin = cast_to_uint8_t_data(mapped_region.get_address());
}

void MappedFile::set_pos(int64_t pos) {
    m_pos = pos > 0 ? pos : 0;
}

int16_t MappedFile::read_int16() {
    return read_next<int16_t>();
}

int32_t MappedFile::read_int32() {
    return read_next<int32_t>();
}

int64_t MappedFile::read_int64() {
    return read_next<int64_t>();
}

int64_t MappedFile::get_pos() {
    return m_pos;
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

#if 0


constexpr int ITERATIONS = 1000;

template <typename T>
void test_arithmetics(T val_to_add) {
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
//            file.write(-1, i);
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


int main1() {
    test_arithmetics<int32_t>(1);
    test_arithmetics<uint32_t>(1);
    test_arithmetics<int64_t>(1);
    test_arithmetics<uint64_t>(1);
    test_arithmetics<float>(1);
    test_arithmetics<double>(1);


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

#endif