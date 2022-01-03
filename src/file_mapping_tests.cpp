#include "io/file_mapping.h"

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
        T tmp = file.read_next_primitive<T>();
        assert(tmp == val_to_add + i);
    }
}


template <typename T, typename V>
void test_basic_strings(T val, to_string<V> converter) {
    std::string fmap = std::string("../file_mapping_text") + std::string(".txt");
    if (fs::exists(fmap)) {
        fs::remove(fmap);
    }
    MappedFile file(fmap, 32);

    // write
    for (auto i = 0; i < ITERATIONS; ++i) {
        T tmp = val + converter(i);
        file.write_next(tmp);
    }

    // read
    file.set_pos(0);
    for (int i = 0; i < ITERATIONS; ++i) {
        auto [data_ptr, size] = file.read_next_data<T, const uint8_t*>();
        T tmp(reinterpret_cast<const V*>(data_ptr), size);
        assert(tmp == val + converter(i));
    }
}

void test_modify_and_save() {
    std::string fmap = std::string("../file_mapping_text") + std::string(".txt");
    {
        MappedFile file(fmap, 32);
        for (int32_t i = 0; i < 1000000; i += 1000) {
            file.write_next(i);
        }
    }
    {
        MappedFile file(fmap, 32);
        for (int32_t i = 0; i < 1000000; ++i) {
            auto anInt = file.read_next_primitive<int32_t>();
            if (i % 1000 == 0) {
                assert(anInt == -1);
            } else {
                assert(i == anInt);
            }
        }
    }
}
//
//void test_array() {
//    std::string fmap = std::string("../file_mapping_array") + std::string(".txt");
//    if (fs::exists(fmap)) {
//        fs::remove(fmap);
//    }
//
//    int n = 1000000;
////    std::vector<int> out(n, 1);
////    std::vector<int> in(n, 0);
//    {
//        MappedFile file(fmap, 32);
////        file.write_int_array(out, n);
//    }
//
//    {
//        MappedFile file(fmap, 32);
////        file.read_int_array(in, n);
//    }
////    assert(in == out);
//}


int main() {
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
        test_basic_strings<std::string>(str, conv_to_str);
    }

    for (auto& wstr : wstrs) {
        test_basic_strings<std::wstring>(wstr, conv_to_wstr);
    }

//    test_basic_strings<std::string>("2ul", t);
//    test_basic_strings<std::wstring>(L"2ul", t1);

//    test_modify_and_save();
//    test_array();

    std::string msg = "File mapping test";
//        BOOST_REQUIRE_MESSAGE(success, msg);
    return 0;
}