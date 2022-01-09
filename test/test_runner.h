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

        static void run(const std::string& db_name, const int order, const int n, std::tuple<K, K, K>& keys_to_remove) {
            TestRunner<K, V> runner(n);
            auto t1 = high_resolution_clock::now();
            runner.test_set(db_name, order, n);
            runner.test_get(db_name, order, n);
            runner.test_remove(db_name, order, n, keys_to_remove);
            runner.test_after_remove(db_name, order, n);
            auto t2 = high_resolution_clock::now();

            /* Getting number of milliseconds as a double. */
            duration<double, std::milli> ms_double = t2 - t1;

            cout << "\tPassed for " + db_name << ": " <<
                 "   added: " << n <<
                 ", found: " << runner.stat.total_found <<
                 ", removed: " << runner.stat.total_removed <<
                 ", total_after_remove: " << runner.stat.total_after_remove <<
                 " in " << ms_double.count() << "ms" << endl;
        }
    private:
        void test_set(const std::string& path, int order, int n) {
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
            assert(stat.all_exist());

            K max_key = verify_map.rbegin()->first + 1;
            for (int i = 0; i < n; ++i)
                stat.total_not_exist += btree.exist(max_key + i);

            assert(stat.any_does_not_exist());
        }

        void test_get(const std::string& path, int order, int n) {
            auto btree = storage.open_volume(path, order);

            for (int i = 0; i < n; ++i) {
                auto actual_value = btree.get(i);
                if (actual_value.has_value()) {
                    check(i, actual_value, verify_map.find(i));
                    stat.total_found++;
                } else {
                    assert(actual_value == std::nullopt);
                    stat.total_not_found++;
                }
            }
            assert(stat.contains_all());
        }

        void test_remove(const std::string& path, int order, int n, std::tuple<int, int, int>& keys_to_remove) {
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
            assert(stat.total_after_remove == static_cast<int64_t>(verify_map.size()));
            assert(stat.found_all_the_remaining());
        }

        void test_after_remove(const std::string& path, int order, int n) {
            auto btree = storage.open_volume(path, order);

            for (int i = 0; i < n; ++i) {
                auto expected_value = verify_map.find(i);
                auto actual_value = btree.get(i);
                if (actual_value.has_value() && expected_value != verify_map.end()) {
                    check(i, actual_value, expected_value);
                    stat.total_after_reopen++;
                } else {
                    assert(actual_value == std::nullopt);
                }
            }
            assert(stat.total_after_reopen == static_cast<int64_t>(verify_map.size()));
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

        static void run(basio::thread_pool& pool, const std::string& db_name, const int order, const int n) {
            TestRunnerMT runner(n);
            auto volume = runner.storage.open_volume(db_name, order);

            auto t1 = high_resolution_clock::now();
            for (int i = 0; i < 10; ++i) {
                runner.fill_map_with_random_values(n);
                runner.test_set(pool, volume, n);
                runner.test_remove(pool, volume, n / 2);
                runner.clear_map();
            }
            auto t2 = high_resolution_clock::now();
            /* Getting number of milliseconds as a double. */
            duration<double, std::milli> ms_double = t2 - t1;
            cout << "\t Passed for " + db_name << ": " << " in " << ms_double.count() << "ms" << endl;
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

        void test_set(basio::thread_pool& pool, VolumeT& volume, const int n) {
            auto task = BoostPackagedTask<TestStat>(boost::bind(&test_set_keys, volume, verify_map, 0, n));
            do_task(pool, task);

            auto get_stat = test_get_keys(volume, 0, n);
            assert(get_stat.total_found == n);
        }

        void test_remove(basio::thread_pool& pool, VolumeT& volume, const int n) {
            auto half = n / 2;
            auto task = BoostPackagedTask<TestStat>(boost::bind(&test_remove_keys, volume, 0, half));

            auto remove_stat = do_task(pool, task);
            assert(remove_stat.total_removed == half);

            auto get_stat = test_get_keys(volume, 0, n);
            assert(get_stat.total_found == half);
            assert(get_stat.total_not_found == half);
        }

        TestStat do_task(basio::thread_pool& pool, BoostPackagedTask<TestStat>& task) {
            auto future = task.get_future();
            post(pool, std::move(task));
            return future.get();
        }

        TestStat test_get_keys(const VolumeT& btree, const int from, const int to) {
            TestStat stat(to - from);
            for (int i = from; i < to; ++i) {
                auto actual_value = btree.get(i);
                if (actual_value.has_value()) {
                    check(i, actual_value, verify_map.find(i));
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