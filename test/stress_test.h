#pragma once

#ifdef UNIT_TESTS

#include <limits>
#include <chrono>

#include "storage.h"
#include "btree_impl/btree_node.h"
#include "utils/error.h"

namespace tests::stress_test {
    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::milliseconds;

namespace {
    constexpr std::string_view output_folder = "../../output_stress_test/";

    int32_t node_size_for_t(const int32_t t) {
        // sizeof(BTreeNode<K,V>::used_keys) = 2
        // sizeof(BTreeNode<K,V>::is_leaf) = 1
        // key_pos.size = (2 * t - 1) * sizeof(pos)
        // child_pos.size = (2 * t)  * sizeof(pos)
        // sizeof(pos) = 8; int64_t
        // total_size = 3 + (4t - 1) * 8;
        return 3 + (4 * t - 1) * 8;
    }

    int32_t get_optimal_tree_order(const int32_t page_size) {
        return ((page_size - 3) / 8 + 1) / 4;
    }

    std::string get_file_name(const std::string& name_part) {
        return output_folder.data() + name_part + ".txt";
    }

    template <typename ValueType, typename VolumeT>
    void run_set(int32_t elements_count, VolumeT& volume) {
        duration<double, std::milli> average_ms_double {};

        ValueGenerator<ValueType> g;
        std::vector<ValueType> values(elements_count);
        std::vector<int32_t> lens(elements_count);
        for (int i = 0; i < elements_count; ++i) {
            const auto& data = g.next_value(i);
            values[i] = data.value;
            lens[i] = data.len;
        }

        auto t1 = high_resolution_clock::now();
        for (int i = 0; i < elements_count; ++i) {
            auto t_avg_1 = high_resolution_clock::now();
            if constexpr(std::is_pointer_v<ValueType>)
                volume.set(i, values[i], lens[i]);
            else
                volume.set(i, values[i]);
            auto t_avg_2 = high_resolution_clock::now();
            average_ms_double += t_avg_2 - t_avg_1;
        }
        auto t2 = high_resolution_clock::now();
        duration<double, std::milli> total_ms_double = t2 - t1;

        cout << "\tstat for volume " << volume.path() << endl;
        cout << "\t\ttotal SET time for " << elements_count << " keys is " << total_ms_double.count() << " ms" << endl;
        cout << "\t\taverage SET time is " << average_ms_double.count() / elements_count << " ms" << endl;
    }

    template <typename K, typename V>
    void run(Storage<K,V>& s, const int32_t order, const int n, const std::string& path, const std::string& type_name) {
        auto v = s.open_volume(path, order);

        cout << "Run set " << type_name << " on " << n << " elements" << endl;
        run_set<V>(n, v);
        cout << endl;
    }
}
    BOOST_AUTO_TEST_SUITE(stress_test, *CleanBeforeTest(output_folder.data()))

        BOOST_AUTO_TEST_CASE(optimal_tree_order) {
            auto page_size = m_boost::bip::mapped_region::get_page_size();
            cout << "System page_size is: " << page_size << " bytes" << endl;

            auto optimal_order = get_optimal_tree_order(page_size);
            cout << "Node size: " << node_size_for_t(optimal_order) << " bytes, it fits to OS page_size" << endl;
            cout << "Optimal tree order: " << optimal_order << std::endl;
        }

        BOOST_AUTO_TEST_CASE(set) {
            int optimal_order = get_optimal_tree_order(m_boost::bip::mapped_region::get_page_size());
            auto elements_count = 1000;

            {
                const auto& path = get_file_name("set_i32_" + std::to_string(optimal_order));
                btree::Storage<int, int> s;
                run(s, optimal_order, elements_count, path, "i32");
            }

            {
                const auto& path = get_file_name("set_i64_" + std::to_string(optimal_order));
                btree::Storage<int, int64_t> s;
                run(s, optimal_order, elements_count, path, "i64");
            }
            {
                const auto& path = get_file_name("set_str_" + std::to_string(optimal_order));
                btree::Storage<int, std::string> s;
                run(s, optimal_order, elements_count, path, "str");
            }
            {
                const auto& path = get_file_name("set_wstr_" + std::to_string(optimal_order));
                btree::Storage<int, std::string> s;
                run(s, optimal_order, elements_count, path, "wstr");
            }
            {
                const auto& path = get_file_name("set_blob_" + std::to_string(optimal_order));
                btree::Storage<int, const char*> s;
                run(s, optimal_order, elements_count, path, "blob");
            }
        }


    BOOST_AUTO_TEST_SUITE_END()
}
#endif // UNIT_TESTS