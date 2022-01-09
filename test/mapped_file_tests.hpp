#ifdef UNIT_TESTS

#include "io/mapped_file.h"

namespace tests {

namespace {
    namespace {
        template <typename T>
        using to_string_ptr = std::basic_string<T> (*)(int i);

        template <typename T>
        using strlen_ptr = size_t(*)(const T *s);

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
    bool run_test_arithmetics() {
        std::string path = "../arithmetics_test.txt";
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
        fs::remove(path);
        return success;
    }

    template <typename T, typename V = typename T::value_type>
    bool run_test_basic_strings(T val, to_string_ptr<V> converter, strlen_ptr<V> strlen_impl) {
        std::string path = "../strings_test.txt";
        int64_t total_write_size = 0;
        int64_t total_read_size = 0;
        bool success = true;
        {
            MappedFile file(path, 32);

            // write
            for (auto i = 0; i < ITERATIONS; ++i) {
                T tmp = val + converter(i);
                int32_t size = calc_size_in_bytes(tmp, strlen_impl);
                file.write_next_data(tmp.data(), size);
            }
            total_write_size = file.get_pos();

            // read
            file.set_pos(0);
            for (int i = 0; i < ITERATIONS; ++i) {
                T tmp = val + converter(i);
                auto[value, size] = file.read_next_data<const uint8_t*>();
                success &= compare(tmp, value, size);
            }
            total_read_size = file.get_pos();
        }

        success &= (total_write_size == total_read_size);
        success &= (fs::file_size(path) == static_cast<uint64_t>(total_write_size));
        fs::remove(path);
        return success;
    }

    bool run_test_modify_and_save() {
        std::string path = "../modify_text.txt";
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
        fs::remove(path);
        return success;
    }

    bool run_test_array() {
        std::string fmap = std::string("../file_mapping_array") + std::string(".txt");
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
        fs::remove(fmap);
        return success;
    }
}
    BOOST_AUTO_TEST_SUITE(mapped_file_test)

    BOOST_AUTO_TEST_CASE(test_arithmetics) {
        bool success = run_test_arithmetics<int32_t>();
        success &= run_test_arithmetics<uint32_t>();
        success &= run_test_arithmetics<int64_t>();
        success &= run_test_arithmetics<uint64_t>();
        success &= run_test_arithmetics<float>();
        success &= run_test_arithmetics<double>();
        BOOST_TEST_REQUIRE(success);
    }

    BOOST_AUTO_TEST_CASE(test_strings) {
        std::string strs[] = { "", "a", "aba", "abacaba", "abba", "abacabacababa" };
        std::wstring wstrs[] = { L"", L"a", L"aba", L"abacaba", L"abba", L"abacabacababa" };

        bool success = true;
        for (auto& str: strs) {
            success &= run_test_basic_strings<std::string>(str, conv_to_str, calc_str_len);
        }

        for (auto& wstr: wstrs) {
            success &= run_test_basic_strings<std::wstring>(wstr, conv_to_wstr, calc_wstr_len);
        }
        BOOST_TEST_REQUIRE(success);
    }

    BOOST_AUTO_TEST_CASE(test_mody_and_save) {
        BOOST_TEST_REQUIRE(run_test_modify_and_save());
    }

    BOOST_AUTO_TEST_CASE(test_array) {
        BOOST_TEST_REQUIRE(run_test_array());
    }

    BOOST_AUTO_TEST_SUITE_END()
}
#endif