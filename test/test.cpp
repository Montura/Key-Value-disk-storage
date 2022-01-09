#ifdef UNIT_TESTS

#include "storage_tests.h"
#include "mapped_file_tests.hpp"

namespace tests {
    using namespace storage_tests;

    BOOST_AUTO_TEST_SUITE(key_value_operations_test)

    BOOST_AUTO_TEST_CASE(test_empty_file) {
        int order = 2;
        bool success = run_test_emtpy_file<int64_t>("empty_s_i32", order);
        success &= run_test_emtpy_file<int64_t>("empty_s_i64", order);
        success &= run_test_emtpy_file<float>("empty_s_f", order);
        success &= run_test_emtpy_file<double>("empty_s_d", order);
        success &= run_test_emtpy_file<std::string>("empty_s_str", order);
        success &= run_test_emtpy_file<std::wstring>("empty_s_wstr", order);
        success &= run_test_emtpy_file<const char*>("empty_s_blob", order);
        BOOST_REQUIRE_MESSAGE(success, "TEST_EMPTY_FILE");
    }

    BOOST_AUTO_TEST_CASE(file_size_after_set_one_element) {
        int order = 2;
        bool success = run_test_file_size_with_one_entry<int32_t>("one_s_i32", order);
        success &= run_test_file_size_with_one_entry<int64_t>("one_s_i64", order);
        success &= run_test_file_size_with_one_entry<float>("one_s_f", order);
        success &= run_test_set_get_one<double>("one_s_d", order);
        success &= run_test_file_size_with_one_entry<std::string>("one_s_str", order);
        success &= run_test_file_size_with_one_entry<std::wstring>("one_s_wstr", order);
        success &= run_test_file_size_with_one_entry<const char*>("one_s_blob", order);
        BOOST_REQUIRE_MESSAGE(success, "TEST_FILE_SIZE");
    }

    BOOST_AUTO_TEST_CASE(set_get_one_element) {
        int order = 2;
        bool success = run_test_set_get_one<int32_t>("get_one_s_i32", order);
        success &= run_test_set_get_one<int64_t>("get_one_s_i64", order);
        success &= run_test_set_get_one<float>("get_one_s_f", order);
        success &= run_test_set_get_one<double>("get_one_s_d", order);
        success &= run_test_set_get_one<std::string>("get_one_s_str", order);
        success &= run_test_set_get_one<std::wstring>("get_one_s_wstr", order);
        success &= run_test_set_get_one<const char*>("get_one_s_blob", order);
        BOOST_REQUIRE_MESSAGE(success, "TEST_SET_GET_ONE_ELEMENT");
    }

    BOOST_AUTO_TEST_CASE(remove_one_element) {
        int order = 2;
        bool success = run_test_remove_one<int32_t>("remove_one_s_i32", order);
        success &= run_test_remove_one<int64_t>("remove_one_s_i64", order);
        success &= run_test_remove_one<float>("remove_one_s_f", order);
        success &= run_test_remove_one<double>("remove_one_s_d", order);
        success &= run_test_remove_one<std::string>("remove_one_s_str", order);
        success &= run_test_remove_one<std::wstring>("remove_one_s_wstr", order);
        success &= run_test_remove_one<const char*>("remove_one_s_blob", order);
        BOOST_REQUIRE_MESSAGE(success, "TEST_REMOVE_ONE_ELEMENT");
    }

    BOOST_AUTO_TEST_CASE(repeatable_operations_on_a_unique_key) {
        int order = 2;
        bool success = run_test_repeatable_operations_on_a_unique_key<int32_t>("repeatable_set_s_i32", order);
        success &= run_test_repeatable_operations_on_a_unique_key<int64_t>("repeatable_set_s_i64", order);
        success &= run_test_repeatable_operations_on_a_unique_key<float>("repeatable_set_s_f", order);
        success &= run_test_repeatable_operations_on_a_unique_key<double>("repeatable_set_s_d", order);
        success &= run_test_repeatable_operations_on_a_unique_key<std::string>("repeatable_set_s_str", order);
        success &= run_test_repeatable_operations_on_a_unique_key<std::wstring>("repeatable_set_s_wstr", order);
        success &= run_test_repeatable_operations_on_a_unique_key<const char*>("repeatable_set_s_blob", order);
        BOOST_REQUIRE_MESSAGE(success, "TEST_REPEATABLE_OPERATIONS");
    }

    BOOST_AUTO_TEST_CASE(set_various_values_on_the_same_key) {
        int order = 2;
        bool success = run_test_set_on_the_same_key<int32_t>("various_set_s_i32", order);
        success &= run_test_set_on_the_same_key<int64_t>("various_set_s_i64", order);
        success &= run_test_set_on_the_same_key<float>("various_set_s_f", order);
        success &= run_test_set_on_the_same_key<double>("various_set_s_d", order);
        success &= run_test_set_on_the_same_key<std::string>("various_set_s_str", order);
        success &= run_test_set_on_the_same_key<std::wstring>("various_set_s_wstr", order);
        success &= run_test_set_on_the_same_key<const char*>("various_set_s_blob", order);
        BOOST_REQUIRE_MESSAGE(success, "TEST_SET_VARIOUS_VALUES");
    }

    BOOST_AUTO_TEST_CASE(test_on_random_values) {
        int order = 2;
        int elements_count = 10000;
        bool success = run_on_random_values<int32_t>("random_s_i32", order, elements_count);
        success &= run_on_random_values<int64_t>("random_s_i64", order, elements_count);
        success &= run_on_random_values<float>("random_s_f", order, elements_count);
        success &= run_on_random_values<double>("random_s_d", order, elements_count);
        success &= run_on_random_values<std::string>("random_s_str", order, elements_count);
        success &= run_on_random_values<std::wstring>("random_s_wstr", order, elements_count);
        success &= run_on_random_values<const char*>("random_s_blob", order, elements_count);
        BOOST_REQUIRE_MESSAGE(success, "TEST_RANDOM_VALUES");
    }

    BOOST_AUTO_TEST_CASE(multithreading_test) {
        int order = 2;
        int elements_count = 10000;
        basio::thread_pool pool(10);
        bool success = run_multithreading_test<int32_t>(pool, "mt_s_i32", order, elements_count);
        success &= run_multithreading_test<int64_t>(pool, "mt_s_i64", order, elements_count);
        success &= run_multithreading_test<float>(pool, "mt_s_f", order, elements_count);
        success &= run_multithreading_test<double>(pool, "mt_s_d", order, elements_count);
        success &= run_multithreading_test<std::string>(pool, "mt_s_str", order, elements_count);
        success &= run_multithreading_test<std::wstring>(pool, "mt_s_wstr", order, elements_count);
        success &= run_multithreading_test<const char*>(pool, "mt_s_blob", order, elements_count);
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
