#include <string>
#include <map>

#include "test_stat.h"
#include "test_utils.h"
#include "storage.h"

namespace btree_test {
    using namespace btree;
    using namespace btree_test::utils;

    template <typename K, typename V>
    class TestRunner {
        TestStat stat;
        std::map<K, V> verify_map;
        Storage<K,V, false> storage;

        explicit TestRunner(int iterations) : stat(iterations) {}

        ~TestRunner() {
            if constexpr(std::is_pointer_v<V>) {
                for (auto& data: verify_map) {
                    delete data.second;
                }
            }
        }
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

            cout << "Passed for " + db_name << ": " <<
                 "\t added: " << n <<
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
                V value = utils::generate_value<V>(i);
                if constexpr(std::is_pointer_v<V>) {
                    btree.set(key, value, utils::get_len_by_idx(i));
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
                auto expected_value = verify_map.find(i);
                auto actual_value = btree.get(i);
                if (actual_value.has_value()) {
                    utils::check(i, actual_value, expected_value);
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
                    if constexpr(std::is_pointer_v<V>) {
                        delete it->second;
                    }
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
                    utils::check(i, actual_value, expected_value);
                    stat.total_after_reopen++;
                } else {
                    assert(actual_value == std::nullopt);
                }
            }
            assert(stat.total_after_reopen == static_cast<int64_t>(verify_map.size()));
        }
    };

}