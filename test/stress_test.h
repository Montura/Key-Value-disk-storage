#pragma once

#ifdef UNIT_TESTS

#include <limits>
#include <chrono>
#include <algorithm>

#include "storage.h"
#include "btree_impl/btree_node.h"
#include "utils/error.h"

namespace tests::stress_test {
    constexpr std::string_view output_folder = "../../output_stress_test/";
    constexpr auto elements_count = 10000000;

namespace details {
    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::milliseconds;
    using std::chrono::time_point;
    using std::chrono::steady_clock;

    // see BTreeNode<K, V>::get_node_size_in_bytes
    constexpr size_t node_size_for_t(const size_t t) {
        return 3 + (4 * t - 1) * 8;
    }

    constexpr int32_t get_optimal_tree_order(const size_t page_size) {
#ifdef _MSC_VER
        return 128; // too big page size: 64 kB for x86|x64
#else
        return static_cast<int32_t>(((page_size - 3) / 8 + 1) / 4);
#endif
    }

    std::string get_file_name(const std::string& name_part, const int32_t optimal_order) {
        return output_folder.data() + name_part + "_" + std::to_string(optimal_order) + ".txt";
    }

    struct TimeStat {
        static double median(std::vector<duration<double, std::milli>>& v) {
            auto mid = v.size() / 2;
            auto m = v.begin() + mid;
            std::nth_element(v.begin(), m, v.end());
            return v[mid].count();
        };

        static double avg(std::vector<duration<double, std::milli>>& v) {
            auto const count = static_cast<float>(v.size());
            return std::reduce(v.begin(), v.end()).count() / count;
        }
    };

    class HRFSize {
        static std::string hrf_size(std::uintmax_t size) {
            std::stringstream ss;
            int i = 0;
            auto mantissa = static_cast<double>(size);
            while (mantissa >= 1024) {
                mantissa /= 1024;
                ++i;
            }
            mantissa = std::ceil(mantissa * 10.) / 10.;
            ss << mantissa << "BKMGTPE"[i];
            if (i > 0)
                ss << "B (" << size << ')';
            return ss.str();
        }

    public:
        static std::string size(const std::string& path) {
            std::error_code ec;
            std::uintmax_t size = fs::file_size(path, ec);
            return ec ? ec.message() : hrf_size(size);
        }
    };

    template<typename K, typename V>
    void run_set(typename Storage<K, V>::VolumeT& volume) {
        std::vector<duration<double, std::milli>> time_points(elements_count);

        ValueGenerator<V> g;
        const int total_rands = 1000;
        Data<V> values[total_rands];
        for (int i = 0; i < total_rands; ++i)
            values[i] = std::move(g.next_value(i));

        int idx = 0;
        auto start = high_resolution_clock::now();
        for (int i = 0; i < elements_count; ++i) {
            idx = i % total_rands;
            auto local_start = high_resolution_clock::now();
            key_value_op_tests::details::set(volume, i, values[idx]);
            time_points[i] = high_resolution_clock::now() - local_start;
        }
        duration<double, std::milli> total_ms_double = high_resolution_clock::now() - start;

        cout << "\t\tSET time -> total is " << total_ms_double.count() << " ms, "
             << "median is " << TimeStat::median(time_points) << " ms, "
             << "average is " << TimeStat::avg(time_points) << " ms" << endl;
    }

    template<typename K, typename V>
    bool run_get(typename Storage<K, V>::VolumeT& volume) {
        std::vector<duration<double, std::milli>> time_points(elements_count);
        bool success = true;

        auto start = high_resolution_clock::now();
        for (int i = 0; i < elements_count; ++i) {
            auto local_start = high_resolution_clock::now();
            success &= volume.get(i).has_value();
            time_points[i] = high_resolution_clock::now() - local_start;
        }
        duration<double, std::milli> total_ms_double = high_resolution_clock::now() - start;

        cout << "\t\tGET time -> total is " << total_ms_double.count() << " ms, "
             << "median is " << TimeStat::median(time_points) << " ms, "
             << "average is " << TimeStat::avg(time_points) << " ms" << endl;
        return success;
    }

    template<typename K, typename V>
    bool run_remove(typename Storage<K, V>::VolumeT& volume) {
        std::vector<duration<double, std::milli>> time_points(elements_count);
        bool success = true;

        auto start = high_resolution_clock::now();
        for (int i = 0; i < elements_count; ++i) {
            auto local_start = high_resolution_clock::now();
            success &= volume.remove(i);
            time_points[i] = high_resolution_clock::now() - local_start;
        }
        duration<double, std::milli> total_ms_double = high_resolution_clock::now() - start;

        cout << "\t\tREMOVE time -> total is " << total_ms_double.count() << " ms, "
             << "median is " << TimeStat::median(time_points) << " ms, "
             << "average is " << TimeStat::avg(time_points) << " ms" << endl;
        return success;
    }
}
    template <typename K, typename V>
    bool run(const std::string& type_name) {
        const int32_t optimal_order = details::get_optimal_tree_order(m_boost::bip::mapped_region::get_page_size());

        const auto& path = details::get_file_name(type_name, optimal_order);
        btree::Storage<K, V> s;
        {
            auto v = s.open_volume(path, optimal_order);
            cout << "\tstat for volume " << v.path() << endl;
            details::run_set<K, V>(v);
        }
        auto max_size = details::HRFSize::size(path);
        auto v = s.open_volume(path, optimal_order);
        bool success = details::run_get<K,V>(v);
        success &= details::run_remove<K,V>(v);
        cout << "\t\tmax file size: " << max_size << endl;
        return success;
    }

    void get_optimal_tree_order() {
        auto page_size = m_boost::bip::mapped_region::get_page_size();
        cout << "System page_size is: " << page_size << " bytes" << endl;

        auto optimal_order = details::get_optimal_tree_order(page_size);
        cout << "\toptimal node size: " << details::node_size_for_t(optimal_order) << " bytes, it fits to OS page_size" << endl;
        cout << "\toptimal tree order: " << optimal_order << std::endl;
    }

    template <typename V>
    bool run(const std::string& type_name) {
        cout << "Run stress_test for type " << type_name << " on " << elements_count << " elements" << endl;
        return run<int, V>(type_name);
    }

}
#endif // UNIT_TESTS