#pragma once

#ifdef UNIT_TESTS

#include "io/mapped_file.h"

namespace tests::mapped_file_test {
    constexpr std::string_view output_folder = "../../output_mapped_file_test/";
    constexpr std::string_view test = "mapped_file_test_";

namespace details {
    std::string get_absolute_file_name(const std::string& name_part) {
        auto name = test.data() + name_part + ".txt";
        return output_folder.data() + name;
    }

    using namespace btree;

    namespace {
        template <typename T>
        using to_string_ptr = std::basic_string<T> (*)(int i);

        to_string_ptr<char> conv_to_str = std::to_string;
        to_string_ptr<wchar_t> conv_to_wstr = std::to_wstring;

        template <typename T>
        bool compare(const T& str, const uint8_t* data, int size) {
            auto raw_data = cast_to_const_uint8_t_data(str.data());
            bool success = true;
            for (int j = 0; j < size; ++j) {
                success &= (data[j] == raw_data[j]);
            }
            return success;
        }
    }

    constexpr int ITERATIONS = 10000;

    template <typename K, typename V>
    bool run_test_arithmetics(const std::string& name_postfix) {
        std::string path = get_absolute_file_name(name_postfix);
        bool success = true;
        {
            MappedFile file(path, 32);
            auto wptr = file.get_mapped_region(0);

            // write
            for (int i = 0; i < ITERATIONS; ++i) {
                V tmp = static_cast<V>(i);
                file.write_next_primitive(wptr, tmp);
            }

            // read
            const auto& ptr = file.get_mapped_region(0);
            for (int i = 0; i < ITERATIONS; ++i) {
                V tmp = file.template read_next_primitive<V>(ptr.get());
                success &= (tmp == static_cast<V>(i));
            }


        }
        success &= (fs::file_size(path) == sizeof(V) * ITERATIONS);
        return success;
    }

    template <typename K, typename V, typename ValueType = typename V::value_type>
    bool run_test_basic_strings(const Data <V>& data, to_string_ptr<ValueType> conv, const std::string& postfix) {
        std::string path = get_absolute_file_name(postfix + "_" + std::to_string(data.len));
        int64_t total_write_size = 0;
        int64_t total_read_size = 0;
        bool success = true;
        {
            MappedFile file(path, 32);

            // write
            for (auto i = 0; i < ITERATIONS; ++i) {
                Data<V> tmp_data(data.value + conv(i));
                file.write_next_data(cast_to_const_uint8_t_data(tmp_data.value.data()), tmp_data.len);
            }
            total_write_size = file.get_pos();

            // read
            const auto& ptr = file.get_mapped_region(0);
            for (int i = 0; i < ITERATIONS; ++i) {
                V tmp = data.value + conv(i);
                auto[value, size] = file.template read_next_data<const uint8_t*>(ptr.get());
                success &= compare(tmp, value, size);
            }
//            total_read_size = file.get_pos();
        }

//        success &= (total_write_size == total_read_size);
        success &= (fs::file_size(path) == static_cast<uint64_t>(total_write_size));
        return success;
    }
}

//    bool run_test_modify_and_save() {
//        using K = int32_t;
//        using V = int32_t;
//        std::string path = details::get_absolute_file_name("modify");
//        bool success = true;
//        {
//            MappedFile file(path, 32);
//            for (int32_t i = 0; i < details::ITERATIONS; ++i) {
//                file.write_next_primitive(i);
//            }
//        }
//        {
//            MappedFile file(path, 32);
//            for (int32_t i = 0; i < details::ITERATIONS; ++i) {
//                file.write_next_primitive(i * 2);
//            }
//        }
//        {
//            MappedFile file(path, 32);
//            const auto& ptr = file.get_mapped_region(0);
//            for (int32_t i = 0; i < details::ITERATIONS; ++i) {
//                auto actual = file.template read_next_primitive<K>(ptr.get());
//                auto expected = i * 2;
//                success &= (actual == expected);
//            }
//        }
//        return success;
//    }
//
//    bool run_test_array() {
//        using K = int32_t;
//        using V = std::vector<K>;
//
//        std::string path = details::get_absolute_file_name("array");
//        const int n = 1000000;
//        std::vector<K> out(n, 1);
//        bool success = false;
//        {
//            MappedFile file(path, 32);
//            int size_in_bytes = n * sizeof(K);
//            file.write_next_data(cast_to_const_uint8_t_data(out.data()), size_in_bytes);
//        }
//
//        {
//            MappedFile file(path, 32);
//            const auto& ptr = file.get_mapped_region(0);
//            auto[data_ptr, size_in_bytes] = file.template read_next_data<const uint8_t*>(ptr.get());
//            auto* int_data = reinterpret_cast<const K*>(data_ptr);
//            std::vector<K> in(int_data, int_data + size_in_bytes / sizeof(K));
//            success = (in == out);
//        }
//        return success;
//    }

    bool run_arithmetic_test() {
        bool success = details::run_test_arithmetics<int32_t, int32_t>("_i32");
        success &= details::run_test_arithmetics<int32_t, uint32_t>("_ui32");
        success &= details::run_test_arithmetics<int32_t, int64_t>("_i64");
        success &= details::run_test_arithmetics<int32_t, uint64_t>("_ui64");
        success &= details::run_test_arithmetics<int32_t, float>("_f");
        success &= details::run_test_arithmetics<int32_t, double>("_d");
        return success;
    }

    bool run_string_test() {
        std::string strs[] = { "", "a", "aba", "abacaba", "abba", "abacabacababa" };
        std::wstring wstrs[] = { L"", L"a", L"aba", L"abacaba", L"abba", L"abacabacababa" };

        bool success = true;
        for (auto& str: strs) {
            Data<std::string> data(str);
            success &= details::run_test_basic_strings<int32_t, std::string>(data, details::conv_to_str, "_str");
        }

        for (auto& w_str: wstrs) {
            Data<std::wstring> data(w_str);
            success &= details::run_test_basic_strings<int32_t, std::wstring>(data, details::conv_to_wstr, "_wstr");
        }
        return success;
    }
}
#endif