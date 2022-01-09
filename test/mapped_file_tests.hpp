#ifdef UNIT_TESTS

#include "io/mapped_file.h"

namespace btree_test {

    BOOST_AUTO_TEST_SUITE(mapped_file_test)
namespace {
    namespace mapped_file_test_utils {
        template <typename T>
        using to_string = std::basic_string<T> (*)(int i);

        template <typename T>
        using strlen = size_t(*)(const T *s);

        to_string<char> conv_to_str = std::to_string;
        to_string<wchar_t> conv_to_wstr = std::to_wstring;

        strlen<char> calc_str_len = std::strlen;
        strlen<wchar_t> calc_wstr_len = std::wcslen;

        template <typename T, typename V = typename T::value_type>
        int32_t calc_size_in_bytes(const T& str, strlen<V> calc_str_len) {
            return sizeof(V) * calc_str_len(str.data());
        }

        template <typename T>
        bool compare(const T& str, const uint8_t* data, int size) {
            auto raw_data = cast_to_const_uint8_t_data(str.data());
            for (int j = 0; j < size; ++j) {
                assert(data[j] == raw_data[j]);
            }
            return true;
        }
    }

    using namespace btree;
    using namespace mapped_file_test_utils;

    constexpr int ITERATIONS = 100;

    template <typename T>
    void run_test_arithmetics() {
        std::string path = "../arithmetics_test.txt";
        {
            MappedFile file(path, 32);

            // write
            for (int i = 0; i < ITERATIONS; ++i) {
                T tmp = i;
                file.write_next_primitive(tmp);
            }

            // read
            file.set_pos(0);
            for (int i = 0; i < ITERATIONS; ++i) {
                T tmp = file.read_next_primitive<T>();
                assert(tmp == static_cast<T>(i));
            }
        }
        assert(fs::file_size(path) == sizeof(T) * ITERATIONS);
        fs::remove(path);
    }

    template <typename T, typename V = typename T::value_type>
    void test_basic_strings(T val, to_string<V> converter, strlen<V> calc_str_len) {
        std::string path = "../strings_test.txt";
        int64_t total_write_size = 0;
        int64_t total_read_size = 0;

        {
            MappedFile file(path, 32);

            // write
            for (auto i = 0; i < ITERATIONS; ++i) {
                T tmp = val + converter(i);
                auto size = calc_size_in_bytes(tmp, calc_str_len);
                file.write_next_data(tmp.data(), size);
            }
            total_write_size = file.get_pos();

            // read
            file.set_pos(0);
            for (int i = 0; i < ITERATIONS; ++i) {
                T tmp = val + converter(i);
                auto[value, size] = file.read_next_data<const uint8_t*>();
                assert(compare(tmp, value, size));
            }
            total_read_size = file.get_pos();
        }

        assert(total_write_size == total_read_size);
        auto size = fs::file_size(path);
        assert(size == static_cast<uint64_t>(total_write_size));
        fs::remove(path);
    }

    void test_modify_and_save() {
        std::string path = "../modify_text.txt";
        {
            MappedFile file(path, 32);
            for (int32_t i = 0; i < 1000000; ++i) {
                file.write_next_primitive(i);
            }
        }
        {
            MappedFile file(path, 32);
            for (int32_t i = 0; i < 1000000; ++i) {
                file.write_next_primitive(i * 2);
            }
        }
        {
            MappedFile file(path, 32);
            for (int32_t i = 0; i < 1000000; ++i) {
                auto actual = file.read_next_primitive<int32_t>();
                auto expected = i * 2;
                assert(actual == expected);
            }
        }
        fs::remove(path);
    }

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
}


    BOOST_AUTO_TEST_CASE(test_arithmetics) {
        run_test_arithmetics<int32_t>();
        run_test_arithmetics<uint32_t>();
        run_test_arithmetics<int64_t>();
        run_test_arithmetics<uint64_t>();
        run_test_arithmetics<float>();
        run_test_arithmetics<double>();
    }

    BOOST_AUTO_TEST_CASE(test_strings) {
        std::string strs[] = { "", "a", "aba", "abacaba", "abba", "abacabacababa" };
        std::wstring wstrs[] = { L"", L"a", L"aba", L"abacaba", L"abba", L"abacabacababa" };

        for (auto& str: strs) {
            test_basic_strings<std::string>(str, conv_to_str, calc_str_len);
        }

        for (auto& wstr: wstrs) {
            test_basic_strings<std::wstring>(wstr, conv_to_wstr, calc_wstr_len);
        }
    }

    BOOST_AUTO_TEST_CASE(test_mody_and_save) {
        test_modify_and_save();
    }

//    test_array();

    BOOST_AUTO_TEST_SUITE_END()
}
#endif