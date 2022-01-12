#pragma once

#ifdef UNIT_TESTS

#include "io/mapped_file.h"

namespace tests::mapped_file_test {
    std::string const output_folder = "../../mapped_file_test_output/";

namespace {
    using namespace btree;

    namespace {
        template <typename T>
        using to_string_ptr = std::basic_string<T> (*)(int i);

        template <typename T>
        using strlen_ptr = size_t(*)(const T* s);

        to_string_ptr<char> conv_to_str = std::to_string;
        to_string_ptr<wchar_t> conv_to_wstr = std::to_wstring;

        strlen_ptr<char> calc_str_len = std::strlen;
        strlen_ptr<wchar_t> calc_wstr_len = std::wcslen;

        template <typename T, typename V = typename T::value_type>
        int32_t calc_size_in_bytes(const T& str, strlen_ptr<V> strlen_impl) {
            return static_cast<int32_t>(sizeof(V) * strlen_impl(str.data()));
        }

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

    template <typename T>
    bool run_test_arithmetics(const std::string& name_postfix) {
        std::string path = output_folder + "mapped_file_test" + name_postfix + ".txt";
        bool success = true;
        {
            MappedFile file(path, 32);

            // write
            for (int i = 0; i < ITERATIONS; ++i) {
                T tmp = static_cast<T>(i);
                file.write_next_primitive(tmp);
            }

            // read
            file.set_pos(0);
            for (int i = 0; i < ITERATIONS; ++i) {
                T tmp = file.read_next_primitive<T>();
                success &= (tmp == static_cast<T>(i));
            }
        }
        success &= (fs::file_size(path) == sizeof(T) * ITERATIONS);
        return success;
    }

    template <typename T, typename V = typename T::value_type>
    bool run_test_basic_strings(T val, to_string_ptr<V> conv, strlen_ptr<V> len_impl, const std::string& postfix) {
        std::string path =
                output_folder + "mapped_file_test_" + postfix + "_" + std::to_string(len_impl(val.data())) + ".txt";
        int64_t total_write_size = 0;
        int64_t total_read_size = 0;
        bool success = true;
        {
            MappedFile file(path, 32);

            // write
            for (auto i = 0; i < ITERATIONS; ++i) {
                T tmp = val + conv(i);
                int32_t size = calc_size_in_bytes(tmp, len_impl);
                file.write_next_data(tmp.data(), size);
            }
            total_write_size = file.get_pos();

            // read
            file.set_pos(0);
            for (int i = 0; i < ITERATIONS; ++i) {
                T tmp = val + conv(i);
                auto[value, size] = file.read_next_data<const uint8_t*>();
                success &= compare(tmp, value, size);
            }
            total_read_size = file.get_pos();
        }

        success &= (total_write_size == total_read_size);
        success &= (fs::file_size(path) == static_cast<uint64_t>(total_write_size));
        return success;
    }

    bool run_test_modify_and_save() {
        std::string path = output_folder + "mapped_file_modify_test.txt";
        bool success = true;
        {
            MappedFile file(path, 32);
            for (int32_t i = 0; i < ITERATIONS; ++i) {
                file.write_next_primitive(i);
            }
        }
        {
            MappedFile file(path, 32);
            for (int32_t i = 0; i < ITERATIONS; ++i) {
                file.write_next_primitive(i * 2);
            }
        }
        {
            MappedFile file(path, 32);
            for (int32_t i = 0; i < ITERATIONS; ++i) {
                auto actual = file.read_next_primitive<int32_t>();
                auto expected = i * 2;
                success &= (actual == expected);
            }
        }
        return success;
    }

    bool run_test_array() {
        std::string fmap = output_folder + "mapped_file_array.txt";
        const int n = 1000000;
        std::vector<int> out(n, 1);
        bool success = false;
        {
            MappedFile file(fmap, 32);
            int size_in_bytes = n * sizeof(int);
            file.write_next_data(out.data(), size_in_bytes);
        }

        {
            MappedFile file(fmap, 32);
            auto [data_ptr, size_in_bytes] = file.read_next_data<const uint8_t*>();
            auto* int_data = reinterpret_cast<const int*>(data_ptr);
            std::vector<int> in(int_data, int_data + size_in_bytes / sizeof(int));
            success = (in == out);
        }
        return success;
    }
}

    BOOST_AUTO_TEST_SUITE(mapped_file_test, *CleanBeforeTest(output_folder))

    BOOST_AUTO_TEST_CASE(test_arithmetics_values) {
        bool success = run_test_arithmetics<int32_t>("_i32");
        success &= run_test_arithmetics<uint32_t>("_ui32");
        success &= run_test_arithmetics<int64_t>("_i64");
        success &= run_test_arithmetics<uint64_t>("_ui64");
        success &= run_test_arithmetics<float>("_f");
        success &= run_test_arithmetics<double>("_d");
        BOOST_REQUIRE_MESSAGE(success, "TEST_ARITHMETICS");
    }

    BOOST_AUTO_TEST_CASE(test_strings_values) {
        std::string strs[] = { "", "a", "aba", "abacaba", "abba", "abacabacababa" };
        std::wstring wstrs[] = { L"", L"a", L"aba", L"abacaba", L"abba", L"abacabacababa" };

        bool success = true;
        for (auto& str: strs)
            success &= run_test_basic_strings<std::string>(str, conv_to_str, calc_str_len, "_str");

        for (auto& wstr: wstrs)
            success &= run_test_basic_strings<std::wstring>(wstr, conv_to_wstr, calc_wstr_len, "_wstr");

        BOOST_REQUIRE_MESSAGE(success, "TEST_STRING");
    }

    BOOST_AUTO_TEST_CASE(test_mody_and_save) {
        BOOST_REQUIRE_MESSAGE(run_test_modify_and_save(), "TEST_MODIFY_AND_SAVE");
    }

    BOOST_AUTO_TEST_CASE(test_array) {
        BOOST_REQUIRE_MESSAGE(run_test_array(), "TEST_ARRAY");
    }

    BOOST_AUTO_TEST_SUITE_END()
}
#endif