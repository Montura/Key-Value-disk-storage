#include <iostream>
#include <string>
#include <map>
#include <chrono>

#include "test_stat.h"
#include "test_utils.h"
#include "storage.h"

namespace tests {
    using std::cout;
    using std::endl;

    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::milliseconds;

    using namespace btree;
    using namespace test_utils;

    template <typename K, typename V>
    class TestRunner {
        TestStat stat;
        std::map<K, V> verify_map;
        Storage<K,V> storage;
        ValueGenerator<V> g;

        explicit TestRunner(int iterations) : stat(iterations) {}

    public:

        static bool run(const std::string& db_name, const int order, const int n, std::tuple<K, K, K>& keys_to_remove) {
            TestRunner<K, V> runner(n);

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
                K key = i;
                V value = g.next_value(key);
                if constexpr(std::is_pointer_v<V>) {
                    btree.set(key, value, get_len_by_idx(i));
                } else {
                    btree.set(key, value);
                }
                verify_map[key] = value;
            }

            for (int i = 0; i < n; ++i)
                stat.total_exist += btree.exist(i);

            K max_key = verify_map.rbegin()->first + 1;
            for (int i = 0; i < n; ++i)
                stat.total_not_exist += btree.exist(max_key + i);

            return stat.all_exist() && stat.any_does_not_exist();
        }

        bool test_get(const std::string& path, int order, int n) {
            auto btree = storage.open_volume(path, order);

            bool success = true;
            for (int i = 0; i < n; ++i) {
                auto actual_value = btree.get(i);
                if (actual_value.has_value()) {
                    success &= check(i, actual_value, verify_map.find(i));
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

            auto[r1, r2, r3] = keys_to_remove;

            auto onErase = [&](const int i) {
                auto it = verify_map.find(i);

                if (it != verify_map.end()) {
                    verify_map.erase(it);
                }
            };

            for (int i = 0; i < n; i += r1) {
                stat.total_removed += btree.remove(i);
                onErase(i);
            }

            for (int i = 0; i < n; i += r2) {
                stat.total_removed += btree.remove(i);
                onErase(i);
            }

            for (int i = 0; i < 50; ++i) {
                onErase(r1);
                stat.total_removed += btree.remove(r1);

                int v2 = 3 * r2;
                onErase(v2);
                stat.total_removed += btree.remove(v2);

                int v3 = 7 * r3;
                onErase(v3);
                stat.total_removed += btree.remove(v3);
            }

            for (int i = 0; i < n; ++i) {
                stat.total_after_remove += btree.exist(i);
            }
            bool success = (stat.total_after_remove == static_cast<int64_t>(verify_map.size()));
            return success && (stat.found_all_the_remaining());
        }

        bool test_after_remove(const std::string& path, int order, int n) {
            auto btree = storage.open_volume(path, order);
            bool success = true;
            for (int i = 0; i < n; ++i) {
                auto expected_value = verify_map.find(i);
                auto actual_value = btree.get(i);
                if (actual_value.has_value() && expected_value != verify_map.end()) {
                    success &= check(i, actual_value, expected_value);
                    stat.total_after_reopen++;
                } else {
                    success &= (actual_value == std::nullopt);
                }
            }
            return success && (stat.total_after_reopen == static_cast<int64_t>(verify_map.size()));
        }
    };

    template <typename K, typename V>
    class TestRunnerMT {
        StorageMT<K,V> storage;
        std::map<K,V> verify_map;
        ValueGenerator<V> g;

        using VolumeT = typename StorageMT<K,V>::VolumeWrapper;
        using VerifyT = void (*)(const TestStat& stat);

        explicit TestRunnerMT(int iterations) {}

    public:

        static bool run(basio::thread_pool& pool, const std::string& db_name, const int order, const int n) {
            TestRunnerMT runner(n);
            auto volume = runner.storage.open_volume(db_name, order);
            bool success = true;

            int run_pool_iterations = 10;
            for (int i = 0; i < run_pool_iterations; ++i) {
                runner.fill_map_with_random_values(n);
                success &= runner.test_set(pool, volume, n);
                success &= runner.test_remove(pool, volume, n / 2);
                runner.clear_map();
            }
#ifdef DEBUG
            cout << "\t Passed for " + db_name << ": in" << run_pool_iterations << " pool iterations " << endl;
#endif
            return success;
        }

    private:
        void fill_map_with_random_values(int n) {
            for (int i = 0; i < n; ++i)
                verify_map[i] = g.next_value(i);
        }

        void clear_map() {
            verify_map.clear();
            g.clear();
        }

        bool test_set(basio::thread_pool& pool, VolumeT& volume, const int n) {
            auto task = BoostPackagedTask<TestStat>(boost::bind(&test_set_keys, volume, verify_map, 0, n));
            do_task(pool, task);

            auto get_stat = test_get_keys(volume, verify_map, 0, n);
            return (get_stat.total_found == n);
        }

        bool test_remove(basio::thread_pool& pool, VolumeT& volume, const int n) {
            auto half = n / 2;
            auto task = BoostPackagedTask<TestStat>(boost::bind(&test_remove_keys, volume, 0, half));

            auto remove_stat = do_task(pool, task);
            bool success = (remove_stat.total_removed == half);

            auto get_stat = test_get_keys(volume, verify_map, 0, n);
            return success && (get_stat.total_found == half) && (get_stat.total_not_found == half);
        }

        TestStat do_task(basio::thread_pool& pool, BoostPackagedTask<TestStat>& task) {
            auto future = task.get_future();
            post(pool, std::move(task));
            return future.get();
        }

        static TestStat test_get_keys(const VolumeT& volume, const std::map<K,V>& verify_map, const int from, const int to) {
            TestStat stat(to - from);
            for (int i = from; i < to; ++i) {
                auto actual_value = volume.get(i);
                if (actual_value.has_value()) {
                    if (check(i, actual_value, verify_map.find(i)))
                        stat.total_found++;
                } else {
                    stat.total_not_found++;
                }
            }
            return stat;
        }

        static TestStat test_set_keys(VolumeT& btree, const std::map<K,V>& verify_map, const int from, const int to) {
            TestStat stat(to - from);
            for (int i = from; i < to; ++i) {
                K key = i;
                V value = verify_map.find(i)->second;
                if constexpr(std::is_pointer_v<V>) {
                    btree.set(key, value, get_len_by_idx(i));
                } else {
                    btree.set(key, value);
                }
            }
            return stat;
        }

        static TestStat test_remove_keys(VolumeT& btree, const int from, const int to) {
            TestStat stat(to - from);
            for (int i = from; i < to; ++i) {
                stat.total_removed += btree.remove(i);
            }
            return stat;
        }

    };
}