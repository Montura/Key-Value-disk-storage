#ifdef UNIT_TESTS

#include "key_value_operations_tests.h"
#include "mapped_file_tests.h"
#include "volume_tests.h"

namespace tests {
    using namespace key_value_op_tests;
    namespace utf = boost::unit_test;

    namespace {
        std::string const output_folder = "../../key-value-test_output/";
        std::string const postfix = ".txt";

        std::string const db_name(const std::string& name, const int order) {
            return output_folder + name + "_" + std::to_string(order) + postfix;
        }
    }

    struct F {
        F()  {
            const std::filesystem::path& p = fs::current_path();
            fs::current_path(p);
            fs::create_directories(output_folder);

#if DEBUG
            BOOST_TEST_MESSAGE( "Setup key_value_op_tests test suite" );
#endif
            try {
                std::uintmax_t count = 0;
                for (auto& de : std::filesystem::directory_iterator(output_folder)) {
                    count += std::filesystem::remove_all(de.path());
                }
#if DEBUG
                std::cout << "deleted " << count << " old files" << std::endl;
#endif
            } catch(const std::exception& ex) {
                std::cerr << ex.what() << std::endl;
            }
        }
//        ~F() { BOOST_TEST_MESSAGE( "teardown fixture" ); }
    };

    BOOST_AUTO_TEST_SUITE(key_value_operations_test, * utf::fixture<F>())

    int const orders[] = { 2, 5, 13, 31, 50, 79, 100};

    BOOST_DATA_TEST_CASE(test_empty_file, boost::make_iterator_range(orders), order) {
        bool success = run_test_emtpy_file<int32_t, int64_t>(db_name("empty_s_i32", order), order);
        success &= run_test_emtpy_file<int32_t, int64_t>(db_name("empty_s_i64", order), order);
        success &= run_test_emtpy_file<int32_t, float>(db_name("empty_s_f", order), order);
        success &= run_test_emtpy_file<int32_t, double>(db_name("empty_s_d", order), order);
        success &= run_test_emtpy_file<int32_t, std::string>(db_name("empty_s_str", order), order);
        success &= run_test_emtpy_file<int32_t, std::wstring>(db_name("empty_s_wstr", order), order);
        success &= run_test_emtpy_file<int32_t, const char*>(db_name("empty_s_blob", order), order);
        BOOST_REQUIRE_MESSAGE(success, "TEST_EMPTY_FILE");
    }

     BOOST_DATA_TEST_CASE(file_size_after_set_one_element, boost::make_iterator_range(orders), order) {
        bool success = run_test_file_size_with_one_entry<int32_t, int32_t>(db_name("one_s_i32", order), order);
        success &= run_test_file_size_with_one_entry<int32_t, int64_t>(db_name("one_s_i64", order), order);
        success &= run_test_file_size_with_one_entry<int32_t, float>(db_name("one_s_f", order), order);
        success &= run_test_set_get_one<int32_t, double>(db_name("one_s_d", order), order);
        success &= run_test_file_size_with_one_entry<int32_t, std::string>(db_name("one_s_str", order), order);
        success &= run_test_file_size_with_one_entry<int32_t, std::wstring>(db_name("one_s_wstr", order), order);
        success &= run_test_file_size_with_one_entry<int32_t, const char*>(db_name("one_s_blob", order), order);
        BOOST_REQUIRE_MESSAGE(success, "TEST_FILE_SIZE");
    }

    BOOST_DATA_TEST_CASE(set_get_one_element, boost::make_iterator_range(orders), order) {
        bool success = run_test_set_get_one<int32_t, int32_t>(db_name("get_one_s_i32", order), order);
        success &= run_test_set_get_one<int32_t, int64_t>(db_name("get_one_s_i64", order), order);
        success &= run_test_set_get_one<int32_t, float>(db_name("get_one_s_f", order), order);
        success &= run_test_set_get_one<int32_t, double>(db_name("get_one_s_d", order), order);
        success &= run_test_set_get_one<int32_t, std::string>(db_name("get_one_s_str", order), order);
        success &= run_test_set_get_one<int32_t, std::wstring>(db_name("get_one_s_wstr", order), order);
        success &= run_test_set_get_one<int32_t, const char*>(db_name("get_one_s_blob", order), order);
        BOOST_REQUIRE_MESSAGE(success, "TEST_SET_GET_ONE_ELEMENT");
    }

    BOOST_DATA_TEST_CASE(remove_one_element, boost::make_iterator_range(orders), order) {
        bool success = run_test_remove_one<int32_t, int32_t>(db_name("remove_one_s_i32", order), order);
        success &= run_test_remove_one<int32_t, int64_t>(db_name("remove_one_s_i64", order), order);
        success &= run_test_remove_one<int32_t, float>(db_name("remove_one_s_f", order), order);
        success &= run_test_remove_one<int32_t, double>(db_name("remove_one_s_d", order), order);
        success &= run_test_remove_one<int32_t, std::string>(db_name("remove_one_s_str", order), order);
        success &= run_test_remove_one<int32_t, std::wstring>(db_name("remove_one_s_wstr", order), order);
        success &= run_test_remove_one<int32_t, const char*>(db_name("remove_one_s_blob", order), order);
        BOOST_REQUIRE_MESSAGE(success, "TEST_REMOVE_ONE_ELEMENT");
    }

    BOOST_DATA_TEST_CASE(repeatable_operations_on_a_unique_key, boost::make_iterator_range(orders), order) {
        bool success = run_test_repeatable_operations_on_a_unique_key<int32_t, int32_t>(db_name("repeatable_set_s_i32", order), order);
        success &= run_test_repeatable_operations_on_a_unique_key<int32_t, int64_t>(db_name("repeatable_set_s_i64", order), order);
        success &= run_test_repeatable_operations_on_a_unique_key<int32_t, float>(db_name("repeatable_set_s_f", order), order);
        success &= run_test_repeatable_operations_on_a_unique_key<int32_t, double>(db_name("repeatable_set_s_d", order), order);
        success &= run_test_repeatable_operations_on_a_unique_key<int32_t, std::string>(db_name("repeatable_set_s_str", order), order);
        success &= run_test_repeatable_operations_on_a_unique_key<int32_t, std::wstring>(db_name("repeatable_set_s_wstr", order), order);
        success &= run_test_repeatable_operations_on_a_unique_key<int32_t, const char*>(db_name("repeatable_set_s_blob", order), order);
        BOOST_REQUIRE_MESSAGE(success, "TEST_REPEATABLE_OPERATIONS");
    }

    BOOST_DATA_TEST_CASE(set_various_values_on_the_same_key, boost::make_iterator_range(orders), order) {
        bool success = run_test_set_on_the_same_key<int32_t, int32_t>(db_name("various_set_s_i32", order), order);
        success &= run_test_set_on_the_same_key<int32_t, int64_t>(db_name("various_set_s_i64", order), order);
        success &= run_test_set_on_the_same_key<int32_t, float>(db_name("various_set_s_f", order), order);
        success &= run_test_set_on_the_same_key<int32_t, double>(db_name("various_set_s_d", order), order);
        success &= run_test_set_on_the_same_key<int32_t, std::string>(db_name("various_set_s_str", order), order);
        success &= run_test_set_on_the_same_key<int32_t, std::wstring>(db_name("various_set_s_wstr", order), order);
        success &= run_test_set_on_the_same_key<int32_t, const char*>(db_name("various_set_s_blob", order), order);
        BOOST_REQUIRE_MESSAGE(success, "TEST_SET_VARIOUS_VALUES");
    }


    BOOST_DATA_TEST_CASE(test_on_random_values, boost::make_iterator_range(orders), order) {
        int elements_count = 10000;
        bool success = run_on_random_values<int32_t, int32_t>(db_name("random_s_i32", order), order, elements_count);
        success &= run_on_random_values<int32_t, int64_t>(db_name("random_s_i64", order), order, elements_count);
        success &= run_on_random_values<int32_t, float>(db_name("random_s_f", order), order, elements_count);
        success &= run_on_random_values<int32_t, double>(db_name("random_s_d", order), order, elements_count);
        success &= run_on_random_values<int32_t, std::string>(db_name("random_s_str", order), order, elements_count);
        success &= run_on_random_values<int32_t, std::wstring>(db_name("random_s_wstr", order), order, elements_count);
        success &= run_on_random_values<int32_t, const char*>(db_name("random_s_blob", order), order, elements_count);
        BOOST_REQUIRE_MESSAGE(success, "TEST_RANDOM_VALUES");
    }

    BOOST_DATA_TEST_CASE(multithreading_test, boost::make_iterator_range(orders), order) {
        int elements_count = 10000;
        ThreadPool pool(10);
        bool success = run_multithreading_test<int32_t, int32_t>(pool, db_name("mt_s_i32", order), order, elements_count);
        success &= run_multithreading_test<int32_t, int64_t>(pool, db_name("mt_s_i64", order), order, elements_count);
        success &= run_multithreading_test<int32_t, float>(pool, db_name("mt_s_f", order), order, elements_count);
        success &= run_multithreading_test<int32_t, double>(pool, db_name("mt_s_d", order), order, elements_count);
        success &= run_multithreading_test<int32_t, std::string>(pool, db_name("mt_s_str", order), order, elements_count);
        success &= run_multithreading_test<int32_t, std::wstring>(pool, db_name("mt_s_wstr", order), order, elements_count);
        success &= run_multithreading_test<int32_t, const char*>(pool, db_name("mt_s_blob", order), order, elements_count);
        BOOST_REQUIRE_MESSAGE(success, "TEST_MULTITHREADING");
    }

    BOOST_AUTO_TEST_SUITE_END()

}

#else

#if defined(MEM_CHECK) && !defined(UNIT_TESTS) && !defined(BOOST_ALL_NO_LIB)
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
    std::vector<int> keys(n), values(n);
    for (int i = 0; i < n; ++i) {
        keys[i] = i; values[i] = -i;
    }

    ThreadPool tp { 10 };
    auto ranges = generate_ranges(n, 10); // ten not-overlapped intervals
    // pass volume to ThreadPool

    std::vector<std::future<bool>> futures;
    for (auto& range : ranges) {
        futures.emplace_back(
                tp.submit([&volume, &keys, &values, &range]() -> bool {
                    for (int i = range.first; i < range.second; ++i)
                        volume.set(keys[i], values[i]);
                    return true;
                })
        );
    }
    for (auto& future : futures) {
        future.get();
    }
    futures.clear();

    // check
    for (int i = 0; i < n; ++i)
        assert(volume.get(i).value() == -i);

    tp.post([&volume, &keys, &n]() {
        for (int i = 0; i < n; ++i) {
            volume.set(keys[i], 0);
        }
    });
    tp.join();

    // check
    for (int i = 0; i < n; ++i)
        assert(volume.get(i).value() == 0);
}

int main() {
#if defined(MEM_CHECK) && !defined(UNIT_TESTS) && !defined(BOOST_ALL_NO_LIB)
    atexit(at_exit_handler);

    int* a = new int[5];
    delete[] a;
    return 0;
#endif
//    usage();
//    mt_usage();


}
#endif // UNIT_TESTS
