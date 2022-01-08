#ifndef UNIT_TESTS
#if USE_BOOST_PREBUILT_STATIC_LIBRARY
#include <boost/test/unit_test.hpp>
#else
#include <boost/test/included/unit_test.hpp>
#endif

namespace {
    BOOST_AUTO_TEST_CASE(test_b_tree_init) {
        const int n = 1000;
        using BTreeIntInt = BTreeStore<int, int>;
        bool found_all = false, any_not_found = false, remove_all = false;
        std::string db_name = "../db_";
        std::string end = ".txt";

        for (int order = 2; order < 101; ++order) {
            int total_deleted = 0;
            {
                BTreeIntInt btree(db_name + std::to_string(order) + end, order);
                for (int i = n - 1; i >= 0; --i) {
                    btree.set(i, 65 + i);
                }

                int total_found = 0;
                for (int i = 0; i < n; ++i) {
                    total_found += btree.exist(i);
                }
                assert(total_found == n);

                int total_not_found = 0;
                const int key_shift = n;
                for (int i = 0; i < n; ++i) {
                    total_not_found += !btree.exist(key_shift + i);
                }
                any_not_found = (total_not_found == n);

                int cycles = 0;
                for (int i = n - 1; i >= 0; i -= 7) {
                    ++cycles;
                    total_deleted += btree.remove(i);
                }
                remove_all = (total_deleted == cycles);

                for (int i = 0; i < n; i += 31) {
                    ++cycles;
                    total_deleted += btree.remove(i);
                }
                remove_all = (total_deleted == cycles);


                total_found = 0;
                for (int i = 0; i < n; ++i) {
                    total_found += btree.exist(i);
                }
                found_all = (total_found == (n -  total_deleted));
            }
            std::string msg = "BTreeStore<int, int, " + std::to_string(order) + ">";
            BOOST_REQUIRE_MESSAGE(found_all && any_not_found && remove_all, msg);

            {
                BTreeIntInt btree(db_name + std::to_string(order) + end, order);
                int total_found = 0;
                for (int i = 0; i < n; ++i) {
                    total_found += btree.exist(i);
                }
                found_all = (total_found == (n - total_deleted));
            }
            msg = "BTreeStore<int, int, " + std::to_string(order) + ">";
            BOOST_REQUIRE_MESSAGE(found_all, msg);

        }
    }
}
#else

#include <iostream>
#include <ctime>
#include <map>
#include <chrono>

using std::cout;
using std::endl;
using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::duration;
using std::chrono::milliseconds;

#include "test_runner.h"
#include "utils/boost_include.h"

#if USE_BOOST_PREBUILT_STATIC_LIBRARY
#include <boost/test/unit_test.hpp>
#else
#include <boost/test/included/unit_test.hpp>
#endif

#include <boost/test/data/test_case.hpp>
//#include <boost/range/iterator_range.hpp>

#include <filesystem>

namespace montura {
namespace test {
    BOOST_AUTO_TEST_SUITE(utf_converters)

namespace {
    using namespace btree;
    using namespace btree_test;
    namespace fs = std::filesystem;

    template <typename VolumeT, typename K, typename V>
    void set(VolumeT& volume, const K& key, const V& val) {
        if constexpr(std::is_pointer_v<V>) {
            int size = strlen(val);
            volume.set(key, val, size);
        } else {
            volume.set(key, val);
        }
    }

    template <typename K, typename V>
    int entry_size_in_file(const K& key, const V& val) {
        if constexpr(std::is_pointer_v<V>) {
            int size = strlen(val);
            return Entry<int32_t,V>(key, val, size).size_in_file();
        } else {
            return Entry<int32_t,V>(key, val).size_in_file();
        }
    }

    template <typename V>
    bool run_test_emtpy_file(std::string const& name, int const order) {
        std::string db_name = "../" + name + ".txt";
        Storage<int32_t, V> s;
        auto v = s.open_volume(db_name, order);
        s.close_volume(v);
        bool success = fs::file_size(db_name) == 0;
        fs::remove(db_name);
        return success;
    }

    template <typename V>
    bool run_test_set_unique(std::string const& name, int const order) {
        std::string db_name = "../" + name + ".txt";
        Storage<int32_t, V> s;
        auto volume = s.open_volume(db_name, order);

        uint32_t total_size = volume.header_size();
        int32_t key = 0;
        V val = utils::generate_value<V>(key);
        total_size += volume.node_size() + entry_size_in_file(key, val);
        set(volume, key, val);

        s.close_volume(volume);
        auto success = fs::file_size(db_name) == total_size;
        fs::remove(db_name);
        return success;
    }

    template <typename V>
    bool run_test_get_one(std::string const& name, int const order) {
        std::string db_name = "../" + name + ".txt";
        Storage<int32_t, V> s;

        int32_t key = 0;
        V expected_val = utils::generate_value<V>(key);
        bool success = false;
        {
            auto volume = s.open_volume(db_name, order);
            set(volume, key, expected_val);
            auto actual_val = volume.get(key);
            success = utils::check(key, actual_val, expected_val);
            s.close_volume(volume);
        }
        {
            auto volume = s.open_volume(db_name, order);
            auto actual_val = volume.get(key);
            success &= utils::check(key, actual_val, expected_val);
            s.close_volume(volume);
        }

        fs::remove(db_name);
        return success;
    }

    template <typename V>
    bool run_test_remove_one(std::string const& name, int const order) {
        std::string db_name = "../" + name + ".txt";
        Storage<int32_t, V> s;

        int32_t key = 0;
        V expected_val = utils::generate_value<V>(key);
        uint32_t total_size = 0;

        bool success = false;
        {
            auto volume = s.open_volume(db_name, order);
            set(volume, key, expected_val);
            success = volume.remove(key);
            total_size = volume.header_size();
            s.close_volume(volume);
        }
        success &= (fs::file_size(db_name) == total_size);
        {
            auto volume = s.open_volume(db_name, order);
            auto actual_val = volume.get(key);
            success &= (actual_val == std::nullopt);
            s.close_volume(volume);
        }
        success &= (fs::file_size(db_name) == total_size);

        fs::remove(db_name);
        return success;
    }

    template <typename V>
    bool run_test_repeatable_operations(std::string const& name, int const order) {
        std::string db_name = "../" + name + ".txt";
        Storage<int32_t, V> s;
        auto volume = s.open_volume(db_name, order);

        int32_t key = 0;
        V expected_val = utils::generate_value<V>(key);
        uint32_t header_size = volume.header_size();
        uint32_t total_size = header_size + volume.node_size() + entry_size_in_file(key, expected_val);

        bool success = true;
        for (int i = 0; i < 100; ++i) {
            set(volume, key, expected_val);
            success &= utils::check(key, volume.get(key), expected_val);
            success &= volume.remove(key);
        }

        s.close_volume(volume);
        success &= fs::file_size(db_name) == header_size;

        fs::remove(db_name);
        return success;
    }
}

    BOOST_AUTO_TEST_CASE(empty_file) {
        int order = 2;
        bool success = run_test_emtpy_file<int64_t>("empty_s_i32", order);
        success &= run_test_emtpy_file<int64_t>("empty_s_i64", order);
        success &= run_test_emtpy_file<float>("empty_s_f", order);
        success &= run_test_emtpy_file<double>("empty_s_d", order);
        success &= run_test_emtpy_file<std::string>("empty_s_str", order);
        success &= run_test_emtpy_file<std::wstring>("empty_s_wstr", order);
        success &= run_test_emtpy_file<const char*>("empty_s_blob", order);
        BOOST_TEST_REQUIRE(success);
    }

    BOOST_AUTO_TEST_CASE(set_one_element) {
        int order = 2;
        bool success = run_test_set_unique<int32_t>("one_s_i32", order);
        success &= run_test_set_unique<int64_t>("one_s_i64", order);
        success &= run_test_set_unique<float>("one_s_f", order);
        success &= run_test_set_unique<double>("one_s_d", order);
        success &= run_test_set_unique<std::string>("one_s_str", order);
        success &= run_test_set_unique<std::wstring>("one_s_wstr", order);
        success &= run_test_set_unique<const char*>("one_s_blob", order);

        BOOST_TEST_REQUIRE(success);
    }

    BOOST_AUTO_TEST_CASE(get_one_element) {
        int order = 2;
        bool success = run_test_get_one<int32_t>("get_one_s_i32", order);
        success &= run_test_get_one<int64_t>("get_one_s_i64", order);
        success &= run_test_get_one<float>("get_one_s_f", order);
        success &= run_test_get_one<double>("get_one_s_d", order);
        success &= run_test_get_one<std::string>("get_one_s_str", order);
        success &= run_test_get_one<std::wstring>("get_one_s_wstr", order);
        success &= run_test_get_one<const char*>("get_one_s_blob", order);

        BOOST_TEST_REQUIRE(success);
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

        BOOST_TEST_REQUIRE(success);
    }

    BOOST_AUTO_TEST_CASE(set_the_element_many_times) {
        int order = 2;
        bool success = run_test_repeatable_operations<int32_t>("repeatable_set_s_i32", order);
        success &= run_test_repeatable_operations<int64_t>("repeatable_set_s_i64", order);
        success &= run_test_set_unique<float>("repeatable_set_s_f", order);
        success &= run_test_set_unique<double>("repeatable_set_s_d", order);
        success &= run_test_set_unique<std::string>("repeatable_set_s_str", order);
        success &= run_test_set_unique<std::wstring>("repeatable_set_s_wstr", order);
        success &= run_test_set_unique<const char*>("repeatable_set_s_blob", order);

        BOOST_TEST_REQUIRE(success);
    }

    BOOST_AUTO_TEST_SUITE_END()

}
}

//void test_set(const std::string& name, const int order) {
//    int iters = 100;
//    Storage<int32_t, int32_t> s;
//    auto v = s.open_volume(name, order);
//
//    for (int i = 0; i < iters; ++i) {
//        v.set(i, i + 65);
//    }
//    for (int i = 0; i < iters; ++i) {
//        auto val = v.get(i);
//        assert(val == i + 65);
//    }
//    for (int i = 0; i < iters; ++i) {
//        v.set(i, -i);
//    }
//    for (int i = 0; i < iters; ++i) {
//        auto val = v.get(i);
//        assert(val == -i);
//    }
//}
//
//

//#include "test_runner.h"
//#include "utils/utils.h"
//using namespace btree_test;
//
//const int n = 10000;
//
//void test() {
//    std::string db_prefix = "../db_";
//    std::string end = ".txt";
//
//    for (int i = 0; i < 11; ++i) {
//        auto keys_to_remove = utils::generate_rand_keys();
//        for (int order = 2; order < 7; ++order) {
//            auto db_name = db_prefix + std::to_string(order);
//            TestRunner<int32_t, int32_t>::run(db_name + "_i32" + end, order, n, keys_to_remove);
//            TestRunner<int32_t, int64_t>::run(db_name + "_i64" + end, order, n, keys_to_remove);
//            TestRunner<int32_t, float>::run(db_name + "_f" + end, order, n, keys_to_remove);
//            TestRunner<int32_t, double>::run(db_name + "_d" + end, order, n, keys_to_remove);
//            TestRunner<int32_t, std::string>::run(db_name + "_str" + end, order, n, keys_to_remove);
//            TestRunner<int32_t, std::wstring>::run(db_name + "_wtr" + end, order, n, keys_to_remove);
//            TestRunner<int32_t, const char*>::run(db_name + "_blob" + end, order, n, keys_to_remove);
//        }
//        std::cout << "iter: " << i << endl;
//    }
//}
//
//void test_mt() {
//    std::string db_prefix = "../db_";
//    std::string end = ".txt";
//
//    basio::thread_pool pool(10);
//
//    for (int i = 0; i < 11; ++i) {
//        for (int order = 2; order < 7; ++order) {
//            auto db_name = db_prefix + std::to_string(order);
//            TestRunnerMT<int32_t, int32_t>::run(pool, db_name + "_i32" + end, order, n);
//            TestRunnerMT<int32_t, int64_t>::run(pool, db_name + "_i64" + end, order, n);
//            TestRunnerMT<int32_t, float>::run(pool, db_name + "_f" + end, order, n);
//            TestRunnerMT<int32_t, double>::run(pool, db_name + "_d" + end, order, n);
//            TestRunnerMT<int32_t, std::string>::run(pool, db_name + "_str" + end, order, n);
//            TestRunnerMT<int32_t, std::wstring>::run(pool, db_name + "_wstr" + end, order, n);
//            TestRunnerMT<int32_t, const char*>::run(pool, db_name + "_blob" + end, order, n);
//        }
//    }
//}
//
//void at_exit_handler();
//
//int main() {
//    std::srand(std::time(nullptr)); // use current time as seed for random generator
//    test();
////    test_mt();
//
//    return 0;
//}


#endif // UNIT_TESTS