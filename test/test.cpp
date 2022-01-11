#ifdef UNIT_TESTS

#include "storage_tests.h"
#include "mapped_file_tests.hpp"

namespace tests {
    using namespace storage_tests;

    BOOST_AUTO_TEST_SUITE(key_value_operations_test)

    int const orders[] = { 2, 5, 13, 31, 50, 79, 100};

    BOOST_DATA_TEST_CASE(test_empty_file, boost::make_iterator_range(orders), order) {
        bool success = run_test_emtpy_file<int32_t, int64_t>("empty_s_i32", order);
        success &= run_test_emtpy_file<int32_t, int64_t>("empty_s_i64", order);
        success &= run_test_emtpy_file<int32_t, float>("empty_s_f", order);
        success &= run_test_emtpy_file<int32_t, double>("empty_s_d", order);
        success &= run_test_emtpy_file<int32_t, std::string>("empty_s_str", order);
        success &= run_test_emtpy_file<int32_t, std::wstring>("empty_s_wstr", order);
        success &= run_test_emtpy_file<int32_t, const char*>("empty_s_blob", order);
        BOOST_REQUIRE_MESSAGE(success, "TEST_EMPTY_FILE");
    }

     BOOST_DATA_TEST_CASE(file_size_after_set_one_element, boost::make_iterator_range(orders), order) {
        bool success = run_test_file_size_with_one_entry<int32_t, int32_t>("one_s_i32", order);
        success &= run_test_file_size_with_one_entry<int32_t, int64_t>("one_s_i64", order);
        success &= run_test_file_size_with_one_entry<int32_t, float>("one_s_f", order);
        success &= run_test_set_get_one<int32_t, double>("one_s_d", order);
        success &= run_test_file_size_with_one_entry<int32_t, std::string>("one_s_str", order);
        success &= run_test_file_size_with_one_entry<int32_t, std::wstring>("one_s_wstr", order);
        success &= run_test_file_size_with_one_entry<int32_t, const char*>("one_s_blob", order);
        BOOST_REQUIRE_MESSAGE(success, "TEST_FILE_SIZE");
    }

    BOOST_DATA_TEST_CASE(set_get_one_element, boost::make_iterator_range(orders), order) {
        bool success = run_test_set_get_one<int32_t, int32_t>("get_one_s_i32", order);
        success &= run_test_set_get_one<int32_t, int64_t>("get_one_s_i64", order);
        success &= run_test_set_get_one<int32_t, float>("get_one_s_f", order);
        success &= run_test_set_get_one<int32_t, double>("get_one_s_d", order);
        success &= run_test_set_get_one<int32_t, std::string>("get_one_s_str", order);
        success &= run_test_set_get_one<int32_t, std::wstring>("get_one_s_wstr", order);
        success &= run_test_set_get_one<int32_t, const char*>("get_one_s_blob", order);
        BOOST_REQUIRE_MESSAGE(success, "TEST_SET_GET_ONE_ELEMENT");
    }

    BOOST_DATA_TEST_CASE(remove_one_element, boost::make_iterator_range(orders), order) {
        bool success = run_test_remove_one<int32_t, int32_t>("remove_one_s_i32", order);
        success &= run_test_remove_one<int32_t, int64_t>("remove_one_s_i64", order);
        success &= run_test_remove_one<int32_t, float>("remove_one_s_f", order);
        success &= run_test_remove_one<int32_t, double>("remove_one_s_d", order);
        success &= run_test_remove_one<int32_t, std::string>("remove_one_s_str", order);
        success &= run_test_remove_one<int32_t, std::wstring>("remove_one_s_wstr", order);
        success &= run_test_remove_one<int32_t, const char*>("remove_one_s_blob", order);
        BOOST_REQUIRE_MESSAGE(success, "TEST_REMOVE_ONE_ELEMENT");
    }

    BOOST_DATA_TEST_CASE(repeatable_operations_on_a_unique_key, boost::make_iterator_range(orders), order) {
        bool success = run_test_repeatable_operations_on_a_unique_key<int32_t, int32_t>("repeatable_set_s_i32", order);
        success &= run_test_repeatable_operations_on_a_unique_key<int32_t, int64_t>("repeatable_set_s_i64", order);
        success &= run_test_repeatable_operations_on_a_unique_key<int32_t, float>("repeatable_set_s_f", order);
        success &= run_test_repeatable_operations_on_a_unique_key<int32_t, double>("repeatable_set_s_d", order);
        success &= run_test_repeatable_operations_on_a_unique_key<int32_t, std::string>("repeatable_set_s_str", order);
        success &= run_test_repeatable_operations_on_a_unique_key<int32_t, std::wstring>("repeatable_set_s_wstr", order);
        success &= run_test_repeatable_operations_on_a_unique_key<int32_t, const char*>("repeatable_set_s_blob", order);
        BOOST_REQUIRE_MESSAGE(success, "TEST_REPEATABLE_OPERATIONS");
    }

    BOOST_DATA_TEST_CASE(set_various_values_on_the_same_key, boost::make_iterator_range(orders), order) {
        bool success = run_test_set_on_the_same_key<int32_t, int32_t>("various_set_s_i32", order);
        success &= run_test_set_on_the_same_key<int32_t, int64_t>("various_set_s_i64", order);
        success &= run_test_set_on_the_same_key<int32_t, float>("various_set_s_f", order);
        success &= run_test_set_on_the_same_key<int32_t, double>("various_set_s_d", order);
        success &= run_test_set_on_the_same_key<int32_t, std::string>("various_set_s_str", order);
        success &= run_test_set_on_the_same_key<int32_t, std::wstring>("various_set_s_wstr", order);
        success &= run_test_set_on_the_same_key<int32_t, const char*>("various_set_s_blob", order);
        BOOST_REQUIRE_MESSAGE(success, "TEST_SET_VARIOUS_VALUES");
    }


    BOOST_DATA_TEST_CASE(test_on_random_values, boost::make_iterator_range(orders), order) {
        int elements_count = 10000;
        bool success = run_on_random_values<int32_t, int32_t>("random_s_i32", order, elements_count);
        success &= run_on_random_values<int32_t, int64_t>("random_s_i64", order, elements_count);
        success &= run_on_random_values<int32_t, float>("random_s_f", order, elements_count);
        success &= run_on_random_values<int32_t, double>("random_s_d", order, elements_count);
        success &= run_on_random_values<int32_t, std::string>("random_s_str", order, elements_count);
        success &= run_on_random_values<int32_t, std::wstring>("random_s_wstr", order, elements_count);
        success &= run_on_random_values<int32_t, const char*>("random_s_blob", order, elements_count);
        BOOST_REQUIRE_MESSAGE(success, "TEST_RANDOM_VALUES");
    }

    BOOST_DATA_TEST_CASE(multithreading_test, boost::make_iterator_range(orders), order) {
        int elements_count = 1000;
        ThreadPool pool(10);
        bool success = run_multithreading_test<int32_t, int32_t>(pool, "mt_s_i32", order, elements_count);
        success &= run_multithreading_test<int32_t, int64_t>(pool, "mt_s_i64", order, elements_count);
        success &= run_multithreading_test<int32_t, float>(pool, "mt_s_f", order, elements_count);
        success &= run_multithreading_test<int32_t, double>(pool, "mt_s_d", order, elements_count);
        success &= run_multithreading_test<int32_t, std::string>(pool, "mt_s_str", order, elements_count);
        success &= run_multithreading_test<int32_t, std::wstring>(pool, "mt_s_wstr", order, elements_count);
        success &= run_multithreading_test<int32_t, const char*>(pool, "mt_s_blob", order, elements_count);
        BOOST_REQUIRE_MESSAGE(success, "TEST_MULTITHREADING");
    }

    BOOST_AUTO_TEST_SUITE_END()

}

#else

#if defined(MEM_CHECK) && !defined(UNIT_TESTS) && !defined(BOOST_ALL_NO_LIB)
    #include <cstdlib>
    #include "utils/mem_util.hpp"
#endif

int main() {
#if defined(MEM_CHECK) && !defined(UNIT_TESTS) && !defined(BOOST_ALL_NO_LIB)
    atexit(at_exit_handler);

    int* a = new int[5];
    delete a;
    return 0;
#endif
}
#endif // UNIT_TESTS
