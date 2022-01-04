#include <string>
#include <map>

#include "test_stat.h"
#include "test_utils.h"
#include "btree.h"

namespace btree_test {
    using namespace btree;
    using namespace btree_test::utils;

    struct TestRunner {
        mutable TestStat stat;

        explicit TestRunner(int iterations) : stat(iterations) {}

        template <typename K, typename V>
        void run(const std::string& db_name, const int order, const int n, std::tuple<K, K, K>& keys_to_remove) {
            auto t1 = high_resolution_clock::now();
            auto value_generator = utils::create_value_generator<V>();
            auto verify_map = test_keys_create_exist<K, V>(db_name, order, n, value_generator);
            std::atomic<int64_t> total_added = verify_map.size();
            test_get_values(db_name, order, n, verify_map);
            test_remove_keys(db_name, order, n, verify_map, keys_to_remove);
            test_after_remove(db_name, order, n, verify_map);
            auto t2 = high_resolution_clock::now();

            /* Getting number of milliseconds as a double. */
            duration<double, std::milli> ms_double = t2 - t1;

            cout << "Passed for " + db_name << ": " <<
                 "\t added: " << total_added <<
                 ", found: " << stat.total_found <<
                 ", removed: " << stat.total_removed <<
                 ", total_after_remove: " << stat.total_after_remove <<
                 " in " << ms_double.count() << "ms" << endl;
            if constexpr(std::is_pointer_v<V>) {
                for (auto& data: verify_map) {
                    delete data.second;
                }
            }
            stat.clear_stat();
        }

    private:
        template <typename K, typename V>
        std::map<K, V> test_keys_create_exist(const std::string& path, int order, int n, utils::generator<V> gen) {
            BTree<K, V> btree(path, order);

            std::map<K, V> verify_map;
            for (int i = 0; i < n; ++i) {
                K key = i;
                V value = gen(i);
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
            return verify_map;
        }

        template <typename K, typename V>
        void test_get_values(const std::string& path, int order, int n, const std::map<K, V>& verify_map) {
            BTree<K, V> btree(path, order);

            for (int i = 0; i < n; ++i) {
                auto expected_value = verify_map.find(i);
                auto actual_value = btree.get(i);
                stat.total_found = utils::check(i, actual_value, expected_value, stat.total_found);
            }
            assert(stat.contains_all());
        }

        template <typename K, typename V>
        void test_remove_keys(const std::string& path, int order, int n, std::map<K, V>& verify_map,
                std::tuple<int, int, int>& keys_to_remove) {
            BTree<K, V> btree(path, order);

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

        template <typename K, typename V>
        void test_after_remove(const std::string& path, int order, int n, const std::map<K, V>& verify_map) {
            BTree<K, V> btree(path, order);

            for (int i = 0; i < n; ++i) {
                auto expected_value = verify_map.find(i);
                auto actual_value = btree.get(i);
                stat.total_after_reopen = utils::check(i, actual_value, expected_value, stat.total_after_reopen);
            }
            assert(stat.total_after_reopen == static_cast<int64_t>(verify_map.size()));
        }
    };

}