#pragma once

#ifdef UNIT_TESTS

#include <iostream>
#include <filesystem>

#include "test_runner/test_runner.h"
#include "utils/size_info.h"

namespace tests {
namespace key_value_op_tests {
    namespace fs = std::filesystem;
    using namespace btree;
    using namespace test_utils;

    template <typename VolumeT, typename K, typename V>
    void set(VolumeT& volume, const K& key, const Data<V>& data) {
        if constexpr(std::is_pointer_v<V>) {
            volume.set(key, data.value, data.len);
        } else {
            volume.set(key, data.value);
        }
    }

    template <typename K, typename V>
    bool run_test_emtpy_file(std::string const& db_name, int const order) {
        {
            Storage<K, V> s;
            s.open_volume(db_name, order);
        }
        bool success = fs::file_size(db_name) == 0;
        return success;
    }

    template <typename K, typename V>
    bool run_test_file_size_with_one_entry(std::string const& db_name, int const order) {
        Storage<K, V> s;
        ValueGenerator<V> g;

        const K key = 0;
        Data<V> data = g.next_value(key);
        uint32_t file_size = SizeInfo<K,V>::file_size_in_bytes(order, key, data.value, data.len);

        auto on_exit = [&](const auto& volume, const uint32_t size_in_bytes) -> bool {
            s.close_volume(volume);
            return (fs::file_size(db_name) == size_in_bytes);
        };

        bool success = true;
        {
            auto volume = s.open_volume(db_name, order);
            set(volume, key, data);
            success &= on_exit(volume, file_size);
        }
        {
            auto volume = s.open_volume(db_name, order);
            success &= g.check(key, volume);
            success &= on_exit(volume, file_size);
        }
        {
            auto volume = s.open_volume(db_name, order);
            success &= volume.remove(key);
            success &= on_exit(volume, SizeInfo<K,V>::header_size_in_bytes());
        }

        return success;
    }

    template <typename K, typename V>
    bool run_test_set_get_one(std::string const& db_name, int const order) {
        Storage<K, V> s;
        ValueGenerator<V> g;

        const K key = 0;
        Data<V> data = g.next_value(key);

        bool success = true;
        {
            auto volume = s.open_volume(db_name, order);
            set(volume, key, data);
            success &= g.check(key, volume);
            s.close_volume(volume);
        }
        {
            auto volume = s.open_volume(db_name, order);
            success &= g.check(key, volume);
            s.close_volume(volume);
        }

        return success;
    }

    template <typename K, typename V>
    bool run_test_remove_one(std::string const& db_name, int const order) {
        Storage<K, V> s;
        ValueGenerator<V> g;

        const K key = 0;
        Data<V> data = g.next_value(key);

        bool success = true;
        {
            auto volume = s.open_volume(db_name, order);
            set(volume, key, data);
            success &= volume.remove(key);
            if (success) {
                g.remove(key);
            }
            s.close_volume(volume);
        }
        success &= (fs::file_size(db_name) == SizeInfo<K, V>::header_size_in_bytes());
        {
            auto volume = s.open_volume(db_name, order);
            success &= g.check(key, volume);
            s.close_volume(volume);
        }
        success &= (fs::file_size(db_name) == SizeInfo<K, V>::header_size_in_bytes());

        return success;
    }

    template <typename K, typename V>
    bool run_test_repeatable_operations_on_a_unique_key(std::string const& db_name, int const order) {
        Storage<K, V> s;
        ValueGenerator<V> g;

        const K key = 0;
        Data<V> data = g.next_value(key);

        auto volume = s.open_volume(db_name, order);
        bool success = true;
        for (int i = 0; i < 100; ++i) {
            set(volume, key, data);
            success &= g.check(key, volume);
            success &= volume.remove(key);
        }

        s.close_volume(volume);
        success &= (fs::file_size(db_name) == SizeInfo<K, V>::header_size_in_bytes());

        return success;
    }

    template <typename K, typename V>
    bool run_test_set_on_the_same_key(std::string const& db_name, int const order) {
        Storage<K, V> s;
        ValueGenerator<V> g;

        const K key = 0;

        auto volume = s.open_volume(db_name, order);
        bool success = true;
        for (int i = 0; i < 1000; ++i) {
            Data<V> data = g.next_value(key);
            set(volume, key, data);
            success &= g.check(key, volume);
        }
        s.close_volume(volume);

        return success;
    }

    template <typename K, typename V>
    bool run_on_random_values(std::string const& db_name, int const order, int const n) {
//        int rounds = 3;
#ifdef DEBUG
        std::cout << "Run " << rounds << " iterations on " << n << " elements: " << std::endl;
#endif
        bool success = true;
//        for (int i = 0; i < rounds; ++i) {
        success &= TestRunner<K, V>::run(db_name, order, n);
//        }
        return success;
    };

    template <typename K, typename V>
    bool run_multithreading_test(ThreadPool& pool, std::string const& db_name, int const order, int const n) {
#ifdef DEBUG
        std::cout << "Run multithreading test on " << n << " elements on 10 threads: " << std::endl;
#endif
        bool success = TestRunnerMT<K, V>::run(pool, db_name, order, n);
        return success;
    };
}
}
#endif // UNIT_TESTS