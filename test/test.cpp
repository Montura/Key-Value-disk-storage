#ifdef UNIT_TESTS

#include <iostream>
#include <filesystem>

#include "test_runner.h"
#include "utils/boost_include.h"

namespace btree_test {
    BOOST_AUTO_TEST_SUITE(key_value_operations)

namespace {
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
        {
            Storage<int32_t, V> s;
            s.open_volume(db_name, order);
        }
        bool success = fs::file_size(db_name) == 0;
        fs::remove(db_name);
        return success;
    }

    template <typename V>
    bool run_test_file_size_with_one_entry(std::string const& name, int const order) {
        std::string db_name = "../" + name + ".txt";

        Storage<int32_t, V> s;
        int32_t key = 0;
        V val = utils::generate_value<V>(key);

        auto on_exit = [](auto& storage, const auto& volume,
                const int32_t key, const V& val, bool after_remove = false) -> bool {
            uint32_t total_size = volume.header_size() +
                                  (after_remove ? 0 : (volume.node_size() + entry_size_in_file(key, val)));
            auto path = volume.path();
            storage.close_volume(volume);
            return (fs::file_size(path) == total_size);
        };

        bool success = true;
        {
            auto volume = s.open_volume(db_name, order);
            set(volume, key, val);
            success &= on_exit(s, volume, key, val);
        }
        {
            auto volume = s.open_volume(db_name, order);
            success &= utils::check(key, volume.get(key), val);
            success &= on_exit(s, volume, key, val);
        }
        {
            auto volume = s.open_volume(db_name, order);
            success &= volume.remove(key);
            success &= on_exit(s, volume, key, val, true);
        }

        fs::remove(db_name);
        return success;
    }

    template <typename V>
    bool run_test_set_get_one(std::string const& name, int const order) {
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
    bool run_test_repeatable_operations_on_a_unique_key(std::string const& name, int const order) {
        std::string db_name = "../" + name + ".txt";
        Storage<int32_t, V> s;
        auto volume = s.open_volume(db_name, order);

        int32_t key = 0;
        V expected_val = utils::generate_value<V>(key);
        uint32_t header_size = volume.header_size();

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

    template <typename V>
    bool run_test_set_on_the_same_key(std::string const& name, int const order) {
        std::string db_name = "../" + name + ".txt";
        Storage<int32_t, V> s;

        auto volume = s.open_volume(db_name, order);
        bool success = true;
        int32_t key = 0;

        for (int i = 0; i < 1000; ++i) {
            V expected_val = utils::generate_value<V>(i);
            set(volume, key, expected_val);
            auto actual_val = volume.get(key);
            success = utils::check(key, actual_val, expected_val);
        }
        s.close_volume(volume);

        fs::remove(db_name);
        return success;
    }

    template <typename V>
    void run_on_random_values(std::string const& name, int const order) {
        std::string db_name = "../" + name + ".txt";
        int rounds = 3;
        int n = 10000;
        cout << "Run " << rounds << " iterations on " << n << " elements: " << endl;
        for (int i = 0; i < rounds; ++i) {
            auto keys_to_remove = utils::generate_rand_keys();
            TestRunner<int32_t, V>::run(db_name, order, n, keys_to_remove);
        }
        fs::remove(db_name);
    };
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

    BOOST_AUTO_TEST_CASE(file_size_after_set_one_element) {
        int order = 2;
        bool success = run_test_file_size_with_one_entry<int32_t>("one_s_i32", order);
        success &= run_test_file_size_with_one_entry<int64_t>("one_s_i64", order);
        success &= run_test_file_size_with_one_entry<float>("one_s_f", order);
        success &= run_test_set_get_one<double>("one_s_d", order);
        success &= run_test_file_size_with_one_entry<std::string>("one_s_str", order);
        success &= run_test_file_size_with_one_entry<std::wstring>("one_s_wstr", order);
        success &= run_test_file_size_with_one_entry<const char*>("one_s_blob", order);

        BOOST_TEST_REQUIRE(success);
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

    BOOST_AUTO_TEST_CASE(repeatable_operations_on_a_unique_key) {
        int order = 2;
        bool success = run_test_repeatable_operations_on_a_unique_key<int32_t>("repeatable_set_s_i32", order);
        success &= run_test_repeatable_operations_on_a_unique_key<int64_t>("repeatable_set_s_i64", order);
        success &= run_test_repeatable_operations_on_a_unique_key<float>("repeatable_set_s_f", order);
        success &= run_test_repeatable_operations_on_a_unique_key<double>("repeatable_set_s_d", order);
        success &= run_test_repeatable_operations_on_a_unique_key<std::string>("repeatable_set_s_str", order);
        success &= run_test_repeatable_operations_on_a_unique_key<std::wstring>("repeatable_set_s_wstr", order);
        success &= run_test_repeatable_operations_on_a_unique_key<const char*>("repeatable_set_s_blob", order);

        BOOST_TEST_REQUIRE(success);
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

        BOOST_TEST_REQUIRE(success);
    }

    BOOST_AUTO_TEST_CASE(test_on_random_values) {
            int order = 2;
            run_on_random_values<int32_t>("random_s_i32", order);
            run_on_random_values<int64_t>("random_s_i64", order);
            run_on_random_values<float>("random_s_f", order);
            run_on_random_values<double>("random_s_d", order);
            run_on_random_values<std::string>("random_s_str", order);
            run_on_random_values<std::wstring>("random_s_wstr", order);
            run_on_random_values<const char*>("random_s_blob", order);

            BOOST_TEST_REQUIRE(true);
        }

    BOOST_AUTO_TEST_SUITE_END()

}
#endif

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

