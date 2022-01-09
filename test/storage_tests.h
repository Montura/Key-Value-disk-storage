#pragma once

#include <iostream>
#include <filesystem>

#include "test_runner.h"

namespace tests {
namespace storage_tests {
    namespace fs = std::filesystem;
    using namespace btree;
    using namespace test_utils;

    template <typename VolumeT, typename K, typename V>
    void set(VolumeT& volume, const K& key, const V& val) {
        if constexpr(std::is_pointer_v<V>) {
            auto size =  static_cast<int32_t>(std::strlen(val));
            volume.set(key, val, size);
        } else {
            volume.set(key, val);
        }
    }

    template <typename K, typename V>
    int32_t entry_size_in_file(const K& key, const V& val) {
        if constexpr(std::is_pointer_v<V>) {
            auto size = static_cast<int32_t>(std::strlen(val));
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
        ValueGenerator<V> g;
        V val = g.next_value(key);

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
            success &= check(key, volume.get(key), val);
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
        ValueGenerator<V> g;
        V expected_val = g.next_value(key);
        bool success = false;
        {
            auto volume = s.open_volume(db_name, order);
            set(volume, key, expected_val);
            auto actual_val = volume.get(key);
            success = check(key, actual_val, expected_val);
            s.close_volume(volume);
        }
        {
            auto volume = s.open_volume(db_name, order);
            auto actual_val = volume.get(key);
            success &= check(key, actual_val, expected_val);
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
        ValueGenerator<V> g;
        V expected_val = g.next_value(key);
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
        ValueGenerator<V> g;
        V expected_val = g.next_value(key);
        uint32_t header_size = volume.header_size();

        bool success = true;
        for (int i = 0; i < 100; ++i) {
            set(volume, key, expected_val);
            success &= check(key, volume.get(key), expected_val);
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

        ValueGenerator<V> g;
        for (int i = 0; i < 1000; ++i) {
            V expected_val = g.next_value(i);
            set(volume, key, expected_val);
            auto actual_val = volume.get(key);
            success &= check(key, actual_val, expected_val);
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
        std::cout << "Run " << rounds << " iterations on " << n << " elements: " << std::endl;
        for (int i = 0; i < rounds; ++i) {
            auto keys_to_remove = generate_rand_keys();
            TestRunner<int32_t, V>::run(db_name, order, n, keys_to_remove);
        }
        fs::remove(db_name);
    };
}
}