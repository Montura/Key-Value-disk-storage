#ifdef UNIT_TESTS

#include "utils/boost_fixture.h"
#include "key_value_operations_tests.h"
#include "mapped_file_tests.h"
#include "volume_tests.h"
#include "stress_test.h"
#include "lru_cache_test.h"
#include "lru_mapped_region_test.h"

namespace tests {
BOOST_AUTO_TEST_SUITE(LRU_test)
    BOOST_DATA_TEST_CASE(test_lru_cache, boost::make_iterator_range(block_size_arr), block_size) {
        BOOST_REQUIRE_MESSAGE(test_lru_single_block(block_size), "TEST_LRU_CACHE_SINGLE_BLOCK");
    }
    BOOST_DATA_TEST_CASE(test_lru_cache_state_for_ranges, boost::make_iterator_range(block_size_arr), block_size) {
        BOOST_REQUIRE_MESSAGE(test_lru_state_for_ranges(block_size), "TEST_LRU_CACHE_STATE_FOR_RANGES");
    }
    BOOST_DATA_TEST_CASE(test_lru_cache_with_fixed_operations_count, boost::make_iterator_range(block_size_arr), block_size) {
        BOOST_REQUIRE_MESSAGE(test_fixed_operations_count(block_size), "TEST_LRU_CACHE_FIXED_OPERATIONS_COUNT");
    }
    BOOST_DATA_TEST_CASE(test_lru_cache_with_random_operations_count, boost::make_iterator_range(block_size_arr), block_size) {
        BOOST_REQUIRE_MESSAGE(test_random_operations_count(block_size), "TEST_LRU_CACHE_RANDOM_OPERATIONS_COUNT");
    }
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(LRU_mapped_file_test)
    BOOST_AUTO_TEST_CASE(test_lru_mapped_file_test) { BOOST_REQUIRE_MESSAGE(test_region(), "TEST_ARITHMETICS"); }
BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(mapped_file_test, *CleanBeforeTest(output_folder.data()))
    BOOST_AUTO_TEST_CASE(test_arithmetics_values) { BOOST_REQUIRE_MESSAGE(run_arithmetic_test(), "TEST_ARITHMETICS"); }
    BOOST_AUTO_TEST_CASE(test_strings_values) { BOOST_REQUIRE_MESSAGE(run_string_test(), "TEST_STRING"); }
    BOOST_AUTO_TEST_CASE(test_mody_and_save) { BOOST_REQUIRE_MESSAGE(run_test_modify_and_save(), "TEST_MODIFY_AND_SAVE"); }
    BOOST_AUTO_TEST_CASE(test_array) { BOOST_REQUIRE_MESSAGE(run_test_array(), "TEST_ARRAY"); }
BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(volume_test, *CleanBeforeTest(output_folder.data()))
    BOOST_AUTO_TEST_CASE(volume_open_close) { BOOST_REQUIRE_MESSAGE(test_volume_open_close(), "TEST_VOLUME_OPEN_CLOSE");}
    BOOST_AUTO_TEST_CASE(volume_order) { BOOST_REQUIRE_MESSAGE(test_volume_order(), "TEST_VOLUME_ORDER");}
    BOOST_AUTO_TEST_CASE(volume_key_size) { BOOST_REQUIRE_MESSAGE(test_volume_key_size(), "TEST_VOLUME_KEY_SIZE");}
    BOOST_AUTO_TEST_CASE(volume_value_type) { BOOST_REQUIRE_MESSAGE(test_volume_type(), "TEST_VOLUME_VALUE"); }
    BOOST_AUTO_TEST_CASE(volume_is_not_shared_between_storages) {
        BOOST_REQUIRE_MESSAGE(test_volume_is_not_shared(), "TEST_VOLUME_IS_NOT_SHARED_BETWEEN_STORAGES");
    }
BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(key_value_op_tests, *CleanBeforeTest(output_folder.data()))
//    BOOST_DATA_TEST_CASE(test_empty_file, boost::make_iterator_range(orders), order) {
//        BOOST_REQUIRE_MESSAGE(run<TestEmptyFile>("empty", order), "TEST_EMPTY_FILE");
//    }
//    BOOST_DATA_TEST_CASE(file_size_after_set_one_element, boost::make_iterator_range(orders), order) {
//        BOOST_REQUIRE_MESSAGE(run<TestFileSizeWithOneEntry>("one_entry", order), "TEST_FILE_SIZE");
//    }
//    BOOST_DATA_TEST_CASE(set_get_one_element, boost::make_iterator_range(orders), order) {
//        BOOST_REQUIRE_MESSAGE(run<TestSetGetOneKey>("set_get_one_entry", order), "TEST_SET_GET_ONE_ELEMENT");
//    }
//    BOOST_DATA_TEST_CASE(remove_one_element, boost::make_iterator_range(orders), order) {
//        BOOST_REQUIRE_MESSAGE(run<TestRemoveOneKey>("remove_one", order), "TEST_REMOVE_ONE_ELEMENT");
//    }
//    BOOST_DATA_TEST_CASE(repeatable_operations_on_a_unique_key, boost::make_iterator_range(orders), order) {
//        BOOST_REQUIRE_MESSAGE(run<TestRepeatableOperationsOnOneKey>("repeatable_ops", order), "TEST_REPEATABLE_OPERATIONS");
//    }
//    BOOST_DATA_TEST_CASE(multiple_set_on_the_same_key, boost::make_iterator_range(orders), order) {
//        BOOST_REQUIRE_MESSAGE(run<TestMultipleSetOnTheSameKey>("multiple_set", order), "TEST_SET_VARIOUS_VALUES");
//    }
//    BOOST_DATA_TEST_CASE(test_on_random_values, boost::make_iterator_range(orders), order) {
//        BOOST_REQUIRE_MESSAGE(run<TestRandomValues>("random", order), "TEST_RANDOM_VALUES");
//    }
//    BOOST_DATA_TEST_CASE(multithreading_test, boost::make_iterator_range(orders), order) {
//        BOOST_REQUIRE_MESSAGE(run<TestMultithreading>("mt", order), "TEST_MULTITHREADING");
//    }
BOOST_AUTO_TEST_SUITE_END()


//BOOST_AUTO_TEST_SUITE(stress_test, *CleanBeforeTest(output_folder.data()))
//    BOOST_AUTO_TEST_CASE(optimal_tree_order) { get_optimal_tree_order(); }
//    BOOST_AUTO_TEST_CASE(i32) { BOOST_REQUIRE_MESSAGE(run<int32_t>("i32"), "TEST_STRESS_INT32"); }
//    BOOST_AUTO_TEST_CASE(i64) { BOOST_REQUIRE_MESSAGE(run<int64_t>("i64"), "TEST_STRESS_INT64"); }
//    BOOST_AUTO_TEST_CASE(str) { BOOST_REQUIRE_MESSAGE(run<std::string>("str"), "TEST_STRESS_STRING"); }
//    BOOST_AUTO_TEST_CASE(wstr) { BOOST_REQUIRE_MESSAGE(run<std::wstring>("wstr"), "TEST_STRESS_WSTRING"); }
//    BOOST_AUTO_TEST_CASE(blob) { BOOST_REQUIRE_MESSAGE(run<const char*>("blob"), "TEST_STRESS_BLOB"); }
//BOOST_AUTO_TEST_SUITE_END()
}
#else

#if defined(MEM_CHECK)
    #include <cstdlib>
    #include "utils/mem_util.h"
#endif

#include "storage.h"
using namespace tests;

void usage() {
    {
//        btree::Storage<std::string, int> int_storage;
//        auto volume = int_storage.open_volume("../int_storage.txt", 2);
//        int val = -1;
//        volume.set("aaa", val);
//        std::optional<int> opt = volume.get("aaa");
//        assert(opt.value() == val);
//        auto ptr = std::make_shared<Block>();
//        heap.push_back(ptr);
//        std::make_heap(heap.begin(), heap.end());
//        hash_table.emplace(ptr->addr, ptr);
//        std::cout << ptr.use_count();
    }
//    std::cout << heap[0].use_count() << "\n";

//    const std::vector<int> data = { 0, 4096, 8192 };
//    auto lower1 = std::lower_bound(data.begin(), data.end(), 0 );
//    auto lower2 = std::lower_bound(data.begin(), data.end(), 4095);
//    auto lower3 = std::lower_bound(data.begin(), data.end(), 4096);
//    auto lower4 = std::lower_bound(data.begin(), data.end(), 4097);
//    auto lower5 = std::lower_bound(data.begin(), data.end(), 8191);
//    auto lower6 = std::lower_bound(data.begin(), data.end(), 8192);
//    auto lower7 = std::lower_bound(data.begin(), data.end(), 8193);
//    std::cout << "l1: " << *lower1 << "\n";
//    std::cout << "l2: " << *lower2 << "\n";
//    std::cout << "l3: " << *lower3 << "\n";
//    std::cout << "l4: " << *lower4 << "\n";
//    std::cout << "l5: " << *lower5 << "\n";
//    std::cout << "l6: " << *lower6 << "\n";
//    std::cout << "l7: " << *lower7 << "\n";
//
//    auto upper1 = std::upper_bound(data.begin(), data.end(), 0 );
//    auto upper2 = std::upper_bound(data.begin(), data.end(), 4095);
//    auto upper3 = std::upper_bound(data.begin(), data.end(), 4096);
//    auto upper4 = std::upper_bound(data.begin(), data.end(), 4097);
//    auto upper5 = std::upper_bound(data.begin(), data.end(), 8191);
//    auto upper6 = std::upper_bound(data.begin(), data.end(), 8192);
//    auto upper7 = std::upper_bound(data.begin(), data.end(), 8193);
//    std::cout << "u1: " << *upper1 << "\n";
//    std::cout << "u2: " << *upper2 << "\n";
//    std::cout << "u3: " << *upper3 << "\n";
//    std::cout << "u4: " << *upper4 << "\n";
//    std::cout << "u5: " << *upper5 << "\n";
//    std::cout << "u6: " << *upper6 << "\n";
//    std::cout << "u7: " << *upper7 << "\n";


//    {
//        btree::Storage<int, std::string> str_storage;
//        auto volume = str_storage.open_volume("../str_storage.txt", 2);
//        std::string val = "abacaba";
//        volume.set(0, val);
//        std::optional<std::string> opt = volume.get(0);
//        assert(opt.value() == val);
//    }
//    {
//        btree::Storage<int, const char*> blob_storage;
//        auto volume = blob_storage.open_volume("../blob_storage.txt", 2);
//        int len = 10;
//        auto blob = std::make_unique<char[]>(len + 1);
//        for (int i = 0; i < len; ++i) {
//            blob[i] = (char)(i + 1);
//        }
//        volume.set(0, blob.get(), len);
//
//        std::optional<const char*> opt = volume.get(0);
//        auto ptr = opt.value();
//        for (int i = 0; i < len; ++i) {
//            assert(ptr[i] == blob[i]);
//        }
//    }
}

#include <iostream>

//std::vector<std::pair<int, int>> generate_ranges(int max_n, int pairs_count) {
//    std::vector<std::pair<int, int>> pairs;
//
//    int step = max_n / pairs_count;
//    pairs.emplace_back(0, step);
//
//    for (int i = 1; i < pairs_count; ++i) {
//        auto end = pairs[i - 1].second;
//        pairs.emplace_back(end, end + step);
//    }
//    pairs[pairs_count - 1].second = max_n;
//
//    for (auto& pair : pairs)
//        std::cout << "{" << pair.first << ", " << pair.second << "}" << " ";
//    std::cout << std::endl;
//    return pairs;
//}

//void mt_usage() {
//    btree::StorageMT<int, int> int_storage;
//    auto volume = int_storage.open_volume("../mt_int_storage.txt", 100);
//
//    int n = 100000;
//    {
//        ThreadPool tp { 10 };
//        auto ranges = generate_ranges(n, 10); // ten not-overlapped intervals
//        for (auto& range: ranges) {
//            tp.post([&volume, &range]() {
//                for (int i = range.first; i < range.second; ++i)
//                    volume.set(i, -i);
//            });
//        }
//        tp.join();
//        for (int i = 0; i < n; ++i)
//            assert(volume.get(i).value() == -i);
//    }
//    {
//        ThreadPool tp { 10 };
//        tp.post([&volume, n]() {
//            for (int i = 0; i < n; ++i)
//                volume.set(i, 0);
//        });
//        tp.join();
//        for (int i = 0; i < n; ++i)
//            assert(volume.get(i).value() == 0);
//    }
//}

int main() {
#if defined(MEM_CHECK)
    atexit(at_exit_handler);
#endif
    {
        test();
//        usage();
//        mt_usage();
    }
}
#endif // UNIT_TESTS
