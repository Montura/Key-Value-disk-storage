#include <string>
#include <map>

#include "btree.h"

using BTreeIntInt = BTree<int32_t, int32_t>;


class TestStat {
    const int64_t N;
public:
    int64_t total_found = 0;
    int64_t total_not_found = 0;
    int64_t total_removed = 0;
    int64_t total_after_remove = 0;
    int64_t total_after_reopen = 0;

    TestStat(int64_t N) : N(N) {}

    bool contains_all() const {
        return total_found == N;
    }

    bool any_not_found() const {
        return total_not_found == 0;
    }

    bool check_deleted(int32_t expected) const {
        return total_removed == expected;
    }

    bool found_all_the_remaining() const {
        return total_after_remove == (N -  total_removed);
    }
};

std::tuple<int, int, int> generate_rand_keys() {
    int r1 = std::rand() % 7 + 1;
    int r2 = std::rand() % 13 + 1;
    int r3 = std::rand() % 17 + 1;
    return std::make_tuple(r1, r2, r3);
}

template <typename K, typename V>
std::map <K, V> test_keys_create_exist(const std::string& path, int order, int total_elements) {
    BTree<K,V> btree(path, order);

    TestStat stat(total_elements);
    std::map <K, V> verify_map;
    for (int i = 0; i < total_elements; ++i) {
        K key = i;
        V value = i + 65;
        btree.set(key, value);
        verify_map[key] = value;
    }

    for (int i = 0; i < total_elements; ++i)
        stat.total_found += btree.exist(i);
    assert(stat.contains_all());

    K max_key = verify_map.rbegin()->first + 1;
    for (int i = 0; i < total_elements; ++i)
        stat.total_not_found += btree.exist(max_key + i);

    assert(stat.any_not_found());
    return verify_map;
}

template <typename K, typename V>
int64_t test_values_get(const std::string& path, int order, int total_elements, const std::map <K, V>& verify_map) {
    BTree<K,V> btree(path, order);

    TestStat stat(total_elements);
    for (int i = 0; i < total_elements; ++i) {
        auto expected_value = verify_map.find(i);
        auto actual_value = btree.get(i);
        assert(expected_value->second == actual_value);
        ++stat.total_found;
    }
    assert(stat.contains_all());
    return stat.total_found;
}

template <typename K, typename V>
std::pair<int64_t, int64_t> test_values_remove(const std::string& path, int order,
                                               int total_elements, std::map <K, V>& verify_map,
                                               std::tuple<int, int, int>& keys_to_remove)
{
    BTree<K,V> btree(path, order);

    auto [r1, r2, r3] = keys_to_remove;

    TestStat stat(total_elements);
    for (int i = 0; i < total_elements; i += r1) {
        stat.total_removed += btree.remove(i);
        verify_map.erase(i);
    }

    for (int i = 0; i < total_elements; i += r2) {
        stat.total_removed += btree.remove(i);
        verify_map.erase(i);
    }

    for (int i = 0; i < 50; ++i) {
        verify_map.erase(r1);
        stat.total_removed += btree.remove(r1);

        int v2 = 3 * r2;
        verify_map.erase(v2);
        stat.total_removed += btree.remove(v2);

        int v3 = 7 * r3;
        verify_map.erase(v3);
        stat.total_removed += btree.remove(v3);
    }

    for (int i = 0; i < total_elements; ++i) {
        stat.total_after_remove += btree.exist(i);
    }
    assert(stat.total_after_remove == static_cast<int64_t>(verify_map.size()));
    assert(stat.found_all_the_remaining());

    return std::make_pair(stat.total_removed, stat.total_after_remove);
}

template <typename K, typename V>
void test_values_after_remove(const std::string& path, int order, int total_elements, const std::map <K, V>& verify_map) {
    BTree<K,V> btree(path, order);

    TestStat stat(total_elements);
    for (int i = 0; i < total_elements; ++i) {
        auto expected_value = verify_map.find(i);
        auto actual_value = btree.get(i);
        if (expected_value == verify_map.end()) {
            assert(actual_value == -1);
        } else {
            assert(expected_value->second == actual_value);
            ++stat.total_after_reopen;
        }
    }
    assert(stat.total_after_reopen == static_cast<int64_t>(verify_map.size()));
}


template <typename K, typename V>
void run(const std::string& db_name, const int order, const int n, std::tuple<K, K, K>& keys_to_remove) {
    auto t1 = high_resolution_clock::now();
    auto verify_map = test_keys_create_exist<K, V>(db_name, order, n);
    auto total_found = test_values_get(db_name, order, n, verify_map);
    auto [total_removed, total_after_remove] = test_values_remove(db_name, order, n, verify_map, keys_to_remove);
    test_values_after_remove(db_name, order, n, verify_map);
    auto t2 = high_resolution_clock::now();

    /* Getting number of milliseconds as a double. */
    duration<double, std::milli> ms_double = t2 - t1;

    cout << "Passed for " + db_name << ": " <<
         "\t added: " << n <<
         ", found: " << total_found <<
         ", removed: " << total_removed <<
         ", total_after_remove: " << total_after_remove <<
         " in " << ms_double.count() << "ms" << endl;
}