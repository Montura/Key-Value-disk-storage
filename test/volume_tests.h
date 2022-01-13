#pragma once

#ifdef UNIT_TESTS

#include "storage.h"
#include "utils/error.h"

namespace tests::volume_test {
    constexpr std::string_view output_folder = "../../output_volume_test/";
    constexpr int order = 2;
    constexpr int key = 0;
    constexpr int value = 123456789;

    std::string get_file_name(const std::string& name_part) {
        return output_folder.data() + name_part + ".txt";
    }

    template <typename K, typename V>
    bool open_to_fail(const std::string& path, const std::string_view& expected_err_msg) {
        try {
            btree::Storage<K,V> s;
            s.open_volume(path, order);
        } catch (const std::logic_error& e) {
            std::string_view actual_err_msg = e.what();
            return actual_err_msg.find(expected_err_msg) != std::string_view::npos;
        }
        return false;
    }
    
    using StorageT = btree::Storage<int, int>;
}
#endif // UNIT_TESTS