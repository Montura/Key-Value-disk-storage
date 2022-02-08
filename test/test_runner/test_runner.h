#include <string>
#include <map>

#include "utils/test_stat.h"
#include "utils/thread_pool.h"
#include "test_value_generator.h"

namespace tests {
    using namespace btree;
    using namespace test_utils;

    template <typename K, typename V>
    class TestRunner {
        TestStat stat;
        Storage<K,V> storage;
        ValueGenerator<K, V> g;

        explicit TestRunner(int iterations) : stat(iterations) {}
    public:
        static bool run(const std::string& db_name, const int order, const int n) {
            TestRunner<K, V> runner {n};

            std::tuple<int, int, int> keys_to_remove  = std::make_tuple(
                runner.g.m_rand() % 7 + 1,
                runner.g.m_rand() % 13 + 2,
                runner.g.m_rand() % 17 + 3
            );

            bool success = runner.test_set(db_name, order, n);
            success &= runner.test_get(db_name, order, n);
            success &= runner.test_remove(db_name, order, n, keys_to_remove);
            success &= runner.test_after_remove(db_name, order, n);

#ifdef DEBUG
            cout << "\tPassed for " + db_name << ": " <<
                 "   added: " << n <<
                 ", found: " << runner.stat.total_found <<
                 ", removed: " << runner.stat.total_removed <<
                 ", total_after_remove: " << runner.stat.total_after_remove << endl;
#endif
            return success;
        }
    private:
        bool test_set(const std::string& path, int order, int n) {
            auto btree = storage.open_volume(path, order);

            for (int i = 0; i < n; ++i) {
                K key = std::to_string(i);
                Data<V> data = g.next_value(key);
                if constexpr(std::is_pointer_v<V>) {
                    btree.set(key, data.value, data.len);
                } else {
                    btree.set(key, data.value);
                }
            }

            for (int i = 0; i < n; ++i) {
                K key = std::to_string(i);
                stat.total_exist += btree.exist(key);
            }

            K max_key = std::to_string(n);
            for (int i = 0; i < n; ++i)
                stat.total_not_exist += btree.exist(max_key + std::to_string(i));

            return stat.all_exist() && stat.any_does_not_exist();
        }

        bool test_get(const std::string& path, int order, int n) {
            auto btree = storage.open_volume(path, order);

            bool success = true;
            for (int i = 0; i < n; ++i) {
                K key = std::to_string(i);
                auto actual_value = btree.get(key);
                if (actual_value.has_value()) {
                    success &= g.check_value(key, actual_value);
                    stat.total_found++;
                } else {
                    success &= (actual_value == std::nullopt);
                    stat.total_not_found++;
                }
            }
            return success && stat.contains_all();
        }

        bool test_remove(const std::string& path, int order, int n, std::tuple<int, int, int>& keys_to_remove) {
            auto btree = storage.open_volume(path, order);

            const auto& [r1, r2, r3] = keys_to_remove;

            for (int i = 0; i < n; i += r1) {
                K key = std::to_string(i);
                stat.total_removed += btree.remove(key);
                g.remove(key);
            }

            for (int i = 0; i < n; i += r2) {
                K key = std::to_string(i);
                stat.total_removed += btree.remove(key);
                g.remove(key);
            }

            for (int i = 0; i < 50; ++i) {
                K key1 = std::to_string(r1);
                g.remove(key1);
                stat.total_removed += btree.remove(key1);

                K v2 = std::to_string(3 * r2);
                g.remove(v2);
                stat.total_removed += btree.remove(v2);

                K v3 = std::to_string(7 * r3);
                g.remove(v3);
                stat.total_removed += btree.remove(v3);
            }

            for (int i = 0; i < n; ++i) {
                K key = std::to_string(i);
                stat.total_after_remove += btree.exist(key);
            }
            bool success = (stat.total_after_remove == g.map_size());
            return success && (stat.found_all_the_remaining());
        }

        bool test_after_remove(const std::string& path, int order, int n) {
            auto btree = storage.open_volume(path, order);
            bool success = true;
            for (int i = 0; i < n; ++i) {
                K key = std::to_string(i);
                auto actual_value = btree.get(key);
                if (actual_value.has_value()) {
                    success &= g.check_value(key, actual_value);
                    stat.total_after_reopen++;
                } else {
                    success &= (actual_value == std::nullopt);
                }
            }
            return success && (stat.total_after_reopen == g.map_size());
        }
    };

//    template <typename K, typename V>
//    class TestRunnerMT {
//        StorageMT<K,V> storage;
//        ValueGenerator<K, V> g;
//
//        explicit TestRunnerMT() {}
//    public:
//        static bool run(ThreadPool& pool, const std::string& db_name, const int order, const int n) {
//            TestRunnerMT runner;
//            auto volume = runner.storage.open_volume(db_name, order);
//            bool success = true;
//
//            runner.fill_map_with_random_values(n);
//            success &= runner.test_set(pool, volume, n);
//            success &= runner.test_remove(pool, volume, n / 2);
//#ifdef DEBUG
//            cout << "\t Passed for " + db_name << endl;
//#endif
//            return success;
//        }
//
//    private:
//        void fill_map_with_random_values(int n) {
//            for (int i = 0; i < n; ++i)
//                g.next_value(i);
//        }
//
//        template <typename VolumeT>
//        bool test_set(ThreadPool& pool, VolumeT& volume, const int n) {
//            auto future = pool.submit([&]() -> TestStat { return test_set_keys(volume, g.map(), 0, n); });
//            future.get();
//
//            auto get_stat = test_get_keys(volume, 0, n);
//            return (get_stat.total_found == n);
//        }
//
//        template <typename VolumeT>
//        bool test_remove(ThreadPool& pool, VolumeT& volume, const int n) {
//            auto half = n / 2;
//
//            auto future = pool.submit([&]() -> TestStat { return test_remove_keys(volume, 0, half); });
//            auto remove_stat = future.get();
//            bool success = (remove_stat.total_removed == half);
//
//            auto get_stat = test_get_keys(volume, 0, n);
//            return success && (get_stat.total_found == half) && (get_stat.total_not_found == half);
//        }
//
//        template <typename VolumeT>
//        TestStat test_get_keys(const VolumeT& volume, const int from, const int to) {
//            TestStat stat(to - from);
//            for (int i = from; i < to; ++i) {
//                auto actual_value = volume.get(i);
//                if (actual_value.has_value()) {
//                    if (g.check_value(i, actual_value))
//                        stat.total_found++;
//                } else {
//                    stat.total_not_found++;
//                }
//            }
//            return stat;
//        }
//
//        template <typename VolumeT>
//        static TestStat test_set_keys(VolumeT& btree, const std::map<K,Data<V>>& verify_map, const int from, const int to) {
//            TestStat stat(to - from);
//            for (int i = from; i < to; ++i) {
//                K key = i;
//                Data<V> data = verify_map.find(i)->second;
//                if constexpr(std::is_pointer_v<V>) {
//                    btree.set(key, data.value, data.len);
//                } else {
//                    btree.set(key, data.value);
//                }
//            }
//            return stat;
//        }
//
//        template <typename VolumeT>
//        static TestStat test_remove_keys(VolumeT& btree, const int from, const int to) {
//            TestStat stat(to - from);
//            for (int i = from; i < to; ++i) {
//                stat.total_removed += btree.remove(i);
//            }
//            return stat;
//        }
//
//    };
}