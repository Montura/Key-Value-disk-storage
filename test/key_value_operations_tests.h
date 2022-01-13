#pragma once

#ifdef UNIT_TESTS

#include <iostream>
#include <filesystem>

#include "test_runner/test_runner.h"
#include "utils/size_info.h"

namespace tests::key_value_op_tests {
    constexpr std::string_view output_folder = "../../output_key_value_op_test/";
    constexpr int orders[] = { 2, 5, 13, 31, 50, 79, 100 };

    namespace fs = std::filesystem;
    using namespace btree;
    using namespace test_utils;

namespace details {
    std::string db_name(const std::string& name, const int tree_order) {
        return output_folder.data() + name + "_" + std::to_string(tree_order) + ".txt";
    }

    template <typename VolumeT, typename K, typename V>
    void set(VolumeT& volume, const K key, const Data<V>& data) {
        if constexpr(std::is_pointer_v<V>) {
            volume.set(key, data.value, data.len);
        } else {
            volume.set(key, data.value);
        }
    }


    struct TestEmptyFile {
        template <typename K, typename V>
        static bool run(std::string const& db_name, int const order) {
            {
                Storage<K, V> s;
                s.open_volume(db_name, order);
            }
            bool success = fs::file_size(db_name) == 0;
            return success;
        }
    };

    struct TestFileSizeWithOneEntry {
        template <typename K, typename V>
        static bool run(std::string const& db_name, int const order) {
            Storage<K, V> s;
            ValueGenerator<V> g;

            const K key = 0;
            Data<V> data = g.next_value(key);
            uint32_t file_size = SizeInfo<K, V>::file_size_in_bytes(order, key, data.value, data.len);

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
                success &= on_exit(volume, SizeInfo<K, V>::header_size_in_bytes());
            }

            return success;
        }
    };

    struct TestSetGetOneKey {
        template <typename K, typename V>
        static bool run(std::string const& db_name, int const order) {
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
    };

    struct TestRemoveOneKey {
        template <typename K, typename V>
        static bool run(std::string const& db_name, int const order) {
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
    };

    struct TestRepeatableOperationsOnOneKey {
        template <typename K, typename V>
        static bool run(std::string const& db_name, int const order) {
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
    };

    struct TestMultipleSetOnTheSameKey {
        template <typename K, typename V>
        static bool run(std::string const& db_name, int const order) {
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
    };

    struct TestRandomValues {
        static constexpr int elements_count = 10000;
        template <typename K, typename V>
        static bool run(std::string const& db_name, int const order) {
            return TestRunner<K, V>::run(db_name, order, elements_count);
        };
    };

    struct TestMultithreading {
        static constexpr int elements_count = 10000;
        static constexpr int workers_count = 10;
        template <typename K, typename V>
        static bool run(std::string const& db_name, int const order) {
            ThreadPool pool(workers_count);
            return TestRunnerMT<K, V>::run(pool, db_name, order, elements_count);
        };
    };
}
    using namespace details;
    template <typename TestClass>
    bool run(const std::string& name_part, const int order) {
        auto name = name_part + "_st";
        bool success = false;
        success = TestClass::template run<int32_t, int32_t>(db_name(name + "_i32", order), order);
        success &= TestClass::template run<int32_t, int64_t>(db_name(name + "_i64", order), order);
        success &= TestClass::template run<int32_t, float>(db_name(name + "_f", order), order);
        success &= TestClass::template run<int32_t, double>(db_name(name + "_d", order), order);
        success &= TestClass::template run<int32_t, std::string>(db_name(name + "_str", order), order);
        success &= TestClass::template run<int32_t, std::wstring>(db_name(name + "_wstr", order), order);
        success &= TestClass::template run<int32_t, const char*>(db_name(name + "_blob", order), order);
        return success;
    }
}
#endif // UNIT_TESTS