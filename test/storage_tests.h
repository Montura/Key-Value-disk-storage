#pragma once

#include <iostream>
#include <filesystem>

#include "test_runner.h"
#include "utils/size_info.h"

namespace tests {
namespace storage_tests {
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
    bool run_test_emtpy_file(std::string const& name, int const order) {
        std::string db_name = "../" + name + ".txt";
        {
            Storage<K, V> s;
            s.open_volume(db_name, order);
        }
        bool success = fs::file_size(db_name) == 0;
        fs::remove(db_name);
        return success;
    }

    template <typename K, typename V>
    bool run_test_file_size_with_one_entry(std::string const& name, int const order) {
        std::string db_name = "../" + name + ".txt";

        Storage<K, V> s;
        const K key = 0;
        ValueGenerator<V> g;

        Data<V> data = g.next_value(key);
        V val = data.value;
        uint32_t file_size = SizeInfo<K,V>::file_size_in_bytes(order, key, val, data.len);

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
            success &= g.check(key, volume.get(key));
            success &= on_exit(volume, file_size);
        }
        {
            auto volume = s.open_volume(db_name, order);
            success &= volume.remove(key);
            success &= on_exit(volume, SizeInfo<K,V>::header_size_in_bytes());
        }

        fs::remove(db_name);
        return success;
    }

    template <typename K, typename V>
    bool run_test_set_get_one(std::string const& name, int const order) {
        std::string db_name = "../" + name + ".txt";
        Storage<K, V> s;

        const K key = 0;
        ValueGenerator<V> g;
        Data<V> data = g.next_value(key);
        bool success = false;
        {
            auto volume = s.open_volume(db_name, order);
            set(volume, key, data);
            auto actual_val = volume.get(key);
            success = g.check(key, actual_val);
            s.close_volume(volume);
        }
        {
            auto volume = s.open_volume(db_name, order);
            auto actual_val = volume.get(key);
            success &= g.check(key, actual_val);
            s.close_volume(volume);
        }

        fs::remove(db_name);
        return success;
    }

    template <typename K, typename V>
    bool run_test_remove_one(std::string const& name, int const order) {
        std::string db_name = "../" + name + ".txt";
        Storage<K, V> s;

        const K key = 0;
        ValueGenerator<V> g;
        Data<V> data = g.next_value(key);
        uint32_t total_size = SizeInfo<K, V>::header_size_in_bytes();

        bool success = false;
        {
            auto volume = s.open_volume(db_name, order);
            set(volume, key, data);
            success = volume.remove(key);
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

    template <typename K, typename V>
    bool run_test_repeatable_operations_on_a_unique_key(std::string const& name, int const order) {
        std::string db_name = "../" + name + ".txt";
        Storage<K, V> s;

        const K key = 0;
        ValueGenerator<V> g;
        Data<V> data = g.next_value(key);
        uint32_t header_size = SizeInfo<K,V>::header_size_in_bytes();

        auto volume = s.open_volume(db_name, order);
        bool success = true;
        for (int i = 0; i < 100; ++i) {
            set(volume, key, data);
            success &= g.check(key, volume.get(key));
            success &= volume.remove(key);
        }

        s.close_volume(volume);
        success &= fs::file_size(db_name) == header_size;

        fs::remove(db_name);
        return success;
    }

    template <typename K, typename V>
    bool run_test_set_on_the_same_key(std::string const& name, int const order) {
        std::string db_name = "../" + name + ".txt";
        Storage<K, V> s;

        auto volume = s.open_volume(db_name, order);
        bool success = true;
        const K key = 0;

        ValueGenerator<V> g;
        for (int i = 0; i < 1000; ++i) {
            Data<V> data = g.next_value(key);
            set(volume, key, data);
            auto actual_val = volume.get(key);
            success &= g.check(key, actual_val);
        }
        s.close_volume(volume);

        fs::remove(db_name);
        return success;
    }

    template <typename K, typename V>
    bool run_on_random_values(std::string const& name, int const order, int const n) {
        std::string db_name = "../" + name + ".txt";
//        int rounds = 3;
#ifdef DEBUG
        std::cout << "Run " << rounds << " iterations on " << n << " elements: " << std::endl;
#endif
        bool success = true;
//        for (int i = 0; i < rounds; ++i) {
        auto keys_to_remove = generate_rand_keys();
        success &= TestRunner<K, V>::run(db_name, order, n, keys_to_remove);
//        }
        fs::remove(db_name);
        return success;
    };

    template <typename K, typename V>
    bool run_multithreading_test(ThreadPool& pool, std::string const& name, int const order, int const n) {
        std::string db_name = "../" + name + ".txt";
#ifdef DEBUG
        std::cout << "Run multithreading test on " << n << " elements on 10 threads: " << std::endl;
#endif
        bool success = TestRunnerMT<K, V>::run(pool, db_name, order, n);
        fs::remove(db_name);
        return success;
    };
}
}