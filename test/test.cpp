#ifdef UNIT_TESTS

#include "utils/boost_fixture.h"
#include "key_value_operations_tests.h"
#include "mapped_file_tests.h"
#include "volume_tests.h"
#include "stress_test.h"

namespace tests {
    using std::cout;
    using std::endl;

    BOOST_AUTO_TEST_SUITE(mapped_file_test, *CleanBeforeTest(output_folder.data()))

        BOOST_AUTO_TEST_CASE(test_arithmetics_values) {
            bool success = run_test_arithmetics<int32_t, int32_t>("_i32");
            success &= run_test_arithmetics<int32_t, uint32_t>("_ui32");
            success &= run_test_arithmetics<int32_t, int64_t>("_i64");
            success &= run_test_arithmetics<int32_t, uint64_t>("_ui64");
            success &= run_test_arithmetics<int32_t, float>("_f");
            success &= run_test_arithmetics<int32_t, double>("_d");
            BOOST_REQUIRE_MESSAGE(success, "TEST_ARITHMETICS");
        }

        BOOST_AUTO_TEST_CASE(test_strings_values) {
            std::string strs[] = { "", "a", "aba", "abacaba", "abba", "abacabacababa" };
            std::wstring wstrs[] = { L"", L"a", L"aba", L"abacaba", L"abba", L"abacabacababa" };

            bool success = true;
            for (auto& str: strs) {
                Data<std::string> data(str);
                success &= run_test_basic_strings<int32_t, std::string>(data, conv_to_str, "_str");
            }

            for (auto& w_str: wstrs) {
                Data<std::wstring> data(w_str);
                success &= run_test_basic_strings<int32_t, std::wstring>(data, conv_to_wstr, "_wstr");
            }

            BOOST_REQUIRE_MESSAGE(success, "TEST_STRING");
        }

        BOOST_AUTO_TEST_CASE(test_mody_and_save) {
            bool success = run_test_modify_and_save<int32_t, int32_t>();
            BOOST_REQUIRE_MESSAGE(success, "TEST_MODIFY_AND_SAVE");
        }

        BOOST_AUTO_TEST_CASE(test_array) {
            bool success = run_test_array<int32_t, std::vector<int32_t>>();
            BOOST_REQUIRE_MESSAGE(success, "TEST_ARRAY");
        }

    BOOST_AUTO_TEST_SUITE_END()

    BOOST_AUTO_TEST_SUITE(volume_test, *CleanBeforeTest(output_folder.data()))

        BOOST_AUTO_TEST_CASE(volume_open_close) {
            const auto& path = get_file_name("volume_open_close");

            StorageT s1;
            auto v1 = s1.open_volume(path, order);
            v1.set(key, value);
            s1.close_volume(v1);

            StorageT s2;
            auto v2 = s2.open_volume(path, order);
            bool success = (v2.get(key) == value);

            BOOST_REQUIRE_MESSAGE(success, "TEST_VOLUME_OPEN_CLOSE");
        }

        BOOST_AUTO_TEST_CASE(volume_order) {
            const auto& path = get_file_name("volume_order_validation");
            bool success = false;
            {
                StorageT s;
                auto v = s.open_volume(path, 100);
                v.set(0, 0);
                s.close_volume(v);
                try {
                    s.open_volume(path, 10);
                } catch (const std::logic_error& e) {
                    std::string_view err_msg = e.what();
                    success = err_msg.find(error_msg::wrong_order_msg) != std::string_view::npos;
                }
            }
            BOOST_REQUIRE_MESSAGE(success, "TEST_VOLUME_ORDER");
        }

        BOOST_AUTO_TEST_CASE(volume_key_size) {
            const auto& path = get_file_name("volume_key_size_validation");
            bool success = false;
            {
                btree::Storage<int32_t, int32_t> s_int32;
                auto v = s_int32.open_volume(path, order);
                v.set(0, 0);
                s_int32.close_volume(v);
                success = open_to_fail<int64_t, int32_t>(path, error_msg::wrong_key_size_msg);
            }
            BOOST_REQUIRE_MESSAGE(success, "TEST_VOLUME_KEY_SIZE");
        }

        BOOST_AUTO_TEST_CASE(volume_value_type) {
            const auto& path = get_file_name("volume_value_validation");
            bool success = false;
            {
                btree::Storage<int32_t, int32_t> s_int32;
                auto v = s_int32.open_volume(path, order);
                v.set(0, 0);
                s_int32.close_volume(v);
                success  = open_to_fail<int32_t, uint32_t>(path, error_msg::wrong_value_type_msg);
                success &= open_to_fail<int32_t, uint64_t>(path, error_msg::wrong_value_type_msg);
                success &= open_to_fail<int32_t, float>(path, error_msg::wrong_value_type_msg);
                success &= open_to_fail<int32_t, double>(path, error_msg::wrong_value_type_msg);
                success &= open_to_fail<int32_t, std::string>(path, error_msg::wrong_value_type_msg);
                success &= open_to_fail<int32_t, std::wstring>(path, error_msg::wrong_value_type_msg);
                success &= open_to_fail<int32_t, const char*>(path, error_msg::wrong_value_type_msg);
            }
            bool elem_size_differs = open_to_fail<int32_t, int64_t>(path, error_msg::wrong_element_size_msg);
            BOOST_REQUIRE_MESSAGE(success && elem_size_differs, "TEST_VOLUME_VALUE");
        }

        BOOST_AUTO_TEST_CASE(volume_is_not_shared_between_storages) {
            const auto& path = get_file_name("volume_is_not_shared");
            bool success = false;
            {
                StorageT s1;
                auto v1 = s1.open_volume(path, order);
                v1.set(key, value);
                {
                    StorageT s2;
                    try {
                        s2.open_volume(path, order);
                    } catch (std::logic_error&) {
                        success = true;
                    }
                }
            }
            StorageT s3;
            auto volume = s3.open_volume(path, order);
            success &= (volume.get(key) == value);

            BOOST_REQUIRE_MESSAGE(success, "TEST_VOLUME_IS_NOT_SHARED_BETWEEN_STORAGES");
        }

    BOOST_AUTO_TEST_SUITE_END()


    BOOST_AUTO_TEST_SUITE(key_value_op_tests, *CleanBeforeTest(output_folder.data()))
        int const orders[] = { 2, 5, 13, 31, 50, 79, 100 };

        BOOST_DATA_TEST_CASE(test_empty_file, boost::make_iterator_range(orders), order) {
            BOOST_REQUIRE_MESSAGE(run<TestEmptyFile>("empty", order), "TEST_EMPTY_FILE");
        }

        BOOST_DATA_TEST_CASE(file_size_after_set_one_element, boost::make_iterator_range(orders), order) {
            BOOST_REQUIRE_MESSAGE(run<TestFileSizeWithOneEntry>("one_entry", order), "TEST_FILE_SIZE");
        }

        BOOST_DATA_TEST_CASE(set_get_one_element, boost::make_iterator_range(orders), order) {
            BOOST_REQUIRE_MESSAGE(run<TestSetGetOneKey>("set_get_one_entry", order), "TEST_SET_GET_ONE_ELEMENT");
        }

        BOOST_DATA_TEST_CASE(remove_one_element, boost::make_iterator_range(orders), order) {
            BOOST_REQUIRE_MESSAGE(run<TestRemoveOneKey>("remove_one", order), "TEST_REMOVE_ONE_ELEMENT");
        }

        BOOST_DATA_TEST_CASE(repeatable_operations_on_a_unique_key, boost::make_iterator_range(orders), order) {
            BOOST_REQUIRE_MESSAGE(run<TestRepeatableOperationsOnOneKey>("repeatable_ops", order),
                    "TEST_REPEATABLE_OPERATIONS");
        }

        BOOST_DATA_TEST_CASE(multiple_set_on_the_same_key, boost::make_iterator_range(orders), order) {
            BOOST_REQUIRE_MESSAGE(run<TestMultipleSetOnTheSameKey>("multiple_set", order),
                    "TEST_SET_VARIOUS_VALUES");
        }

        BOOST_DATA_TEST_CASE(test_on_random_values, boost::make_iterator_range(orders), order) {
            int elements_count = 10000;
            BOOST_REQUIRE_MESSAGE(run<TestRandomValues>("random", order, elements_count), "TEST_RANDOM_VALUES");
        }

        BOOST_DATA_TEST_CASE(multithreading_test, boost::make_iterator_range(orders), order) {
            int elements_count = 10000;
            ThreadPool pool(10);
            BOOST_REQUIRE_MESSAGE(run<TestMultithreading>("mt", order, elements_count, pool),
                    "TEST_MULTITHREADING");
        }

    BOOST_AUTO_TEST_SUITE_END()

    BOOST_AUTO_TEST_SUITE(stress_test, *CleanBeforeTest(output_folder.data()))

        BOOST_AUTO_TEST_CASE(optimal_tree_order) {
            auto page_size = m_boost::bip::mapped_region::get_page_size();
            cout << "System page_size is: " << page_size << " bytes" << endl;

            auto optimal_order = get_optimal_tree_order(page_size);
            cout << "Node size: " << node_size_for_t(optimal_order) << " bytes, it fits to OS page_size" << endl;
            cout << "Optimal tree order: " << optimal_order << std::endl;
        }

        BOOST_AUTO_TEST_CASE(i32) {
            cout << "Run stress_test for type i32 on " << elements_count << " elements" << endl;
            bool success = run<int, int32_t>("i32");
            BOOST_REQUIRE_MESSAGE(success, "TEST_STRESS_INT32");
        }

        BOOST_AUTO_TEST_CASE(i64) {
            cout << "Run stress_test for type i64 on " << elements_count << " elements" << endl;
            bool success = run<int, int64_t>("i64");
            BOOST_REQUIRE_MESSAGE(success, "TEST_STRESS_INT64");
        }

        BOOST_AUTO_TEST_CASE(str) {
            cout << "Run stress_test for type str on " << elements_count << " elements" << endl;
            bool success = run<int, std::string>("str");
            BOOST_REQUIRE_MESSAGE(success, "TEST_STRESS_STRING");
        }

        BOOST_AUTO_TEST_CASE(wstr) {
            cout << "Run stress_test for type wstr on " << elements_count << " elements" << endl;
            bool success = run<int, std::wstring>("wstr");
            BOOST_REQUIRE_MESSAGE(success, "TEST_STRESS_WSTRING");
        }

        BOOST_AUTO_TEST_CASE(blob) {
            cout << "Run stress_test for type blob on " << elements_count << " elements" << endl;
            bool success = run<int, const char*>("blob");
            BOOST_REQUIRE_MESSAGE(success, "TEST_STRESS_BLOB");
        }

    BOOST_AUTO_TEST_SUITE_END()
}
#else

#if defined(MEM_CHECK)
    #include <cstdlib>
    #include "utils/mem_util.h"
#endif

#include "storage.h"
#include "utils/thread_pool.h"

using namespace tests;
void usage() {
    {
        btree::Storage<int, int> int_storage;
        auto volume = int_storage.open_volume("../int_storage.txt", 2);
        int val = -1;
        volume.set(0, val);
        std::optional<int> opt = volume.get(0);
        assert(opt.value() == val);
    }
    {
        btree::Storage<int, std::string> str_storage;
        auto volume = str_storage.open_volume("../str_storage.txt", 2);
        std::string val = "abacaba";
        volume.set(0, val);
        std::optional<std::string> opt = volume.get(0);
        assert(opt.value() == val);
    }
    {
        btree::Storage<int, const char*> blob_storage;
        auto volume = blob_storage.open_volume("../blob_storage.txt", 2);
        int len = 10;
        auto blob = std::make_unique<char*>(new char[len + 1]);
        for (int i = 0; i < len; ++i) {
            (*blob)[i] = (char)(i + 1);
        }
        volume.set(0, *blob, len);

        std::optional<const char*> opt = volume.get(0);
        auto ptr = opt.value();
        for (int i = 0; i < len; ++i) {
            assert(ptr[i] == (*blob)[i]);
        }
    }
}

#include <iostream>

std::vector<std::pair<int, int>> generate_ranges(int max_n, int pairs_count) {
    std::vector<std::pair<int, int>> pairs;

    int step = max_n / pairs_count;
    pairs.emplace_back(0, step);

    for (int i = 1; i < pairs_count; ++i) {
        auto end = pairs[i - 1].second;
        pairs.emplace_back(end, end + step);
    }
    pairs[pairs_count - 1].second = max_n;

    for (auto& pair : pairs)
        std::cout << "{" << pair.first << ", " << pair.second << "}" << " ";
    std::cout << std::endl;
    return pairs;
}

void mt_usage() {
    btree::StorageMT<int, int> int_storage;
    auto volume = int_storage.open_volume("../mt_int_storage.txt", 100);

    int n = 100000;
    {
        ThreadPool tp { 10 };
        auto ranges = generate_ranges(n, 10); // ten not-overlapped intervals
        for (auto& range: ranges) {
            tp.post([&volume, &range]() {
                for (int i = range.first; i < range.second; ++i)
                    volume.set(i, -i);
            });
        }
        tp.wait();
        for (int i = 0; i < n; ++i)
            assert(volume.get(i).value() == -i);
    }
    {
        ThreadPool tp { 10 };
        tp.post([&volume, &n]() {
            for (int i = 0; i < n; ++i)
                volume.set(i, 0);
        });
        tp.wait();
        for (int i = 0; i < n; ++i)
            assert(volume.get(i).value() == 0);
    }
}

int main() {
#if defined(MEM_CHECK)
    atexit(at_exit_handler);
#endif
    {
        usage();
        mt_usage();
    }
}
#endif // UNIT_TESTS
