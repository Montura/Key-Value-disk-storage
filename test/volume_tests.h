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

    BOOST_AUTO_TEST_SUITE(volume_tests, *CleanBeforeTest(volume_test::output_folder.data()))

    BOOST_AUTO_TEST_CASE(volume_open_close) {
        const auto& path = get_file_name("volume_open_close");

        StorageT s1;
        auto v1 = s1.open_volume(path, order);
        v1.set(key, value);
        s1.close_volume(v1);

        StorageT s2;
        auto v2 = s2.open_volume(path, order);
        bool success = (v2.get(key) == value);

        BOOST_REQUIRE_MESSAGE(success, "TEST_VOLUME_OPEN_CLOSE");
    }

    BOOST_AUTO_TEST_CASE(volume_order) {
        const auto& path = get_file_name("volume_order_validation");
        bool success = false;
        {
            StorageT s;
            auto v = s.open_volume(path, 100);
            v.set(0, 0);
            s.close_volume(v);
            try {
                s.open_volume(path, 10);
            } catch (const std::logic_error& e) {
                std::string_view err_msg = e.what();
                success = err_msg.find(error_msg::wrong_order_msg) != std::string_view::npos;
            }
        }
        BOOST_REQUIRE_MESSAGE(success, "TEST_VOLUME_ORDER");
    }

    BOOST_AUTO_TEST_CASE(volume_key_size) {
        const auto& path = get_file_name("volume_key_size_validation");
        bool success = false;
        {
            btree::Storage<int32_t, int32_t> s_int32;
            auto v = s_int32.open_volume(path, order);
            v.set(0, 0);
            s_int32.close_volume(v);
            success = open_to_fail<int64_t, int32_t>(path, error_msg::wrong_key_size_msg);
        }
        BOOST_REQUIRE_MESSAGE(success, "TEST_VOLUME_KEY_SIZE");
    }

    BOOST_AUTO_TEST_CASE(volume_value_type) {
        const auto& path = get_file_name("volume_value_validation");
        bool success = false;
        {
            btree::Storage<int32_t, int32_t> s_int32;
            auto v = s_int32.open_volume(path, order);
            v.set(0, 0);
            s_int32.close_volume(v);
            success  = open_to_fail<int32_t, uint32_t>(path, error_msg::wrong_value_type_msg);
            success &= open_to_fail<int32_t, uint64_t>(path, error_msg::wrong_value_type_msg);
            success &= open_to_fail<int32_t, float>(path, error_msg::wrong_value_type_msg);
            success &= open_to_fail<int32_t, double>(path, error_msg::wrong_value_type_msg);
            success &= open_to_fail<int32_t, std::string>(path, error_msg::wrong_value_type_msg);
            success &= open_to_fail<int32_t, std::wstring>(path, error_msg::wrong_value_type_msg);
            success &= open_to_fail<int32_t, const char*>(path, error_msg::wrong_value_type_msg);
        }
        bool elem_size_differs = open_to_fail<int32_t, int64_t>(path, error_msg::wrong_element_size_msg);
        BOOST_REQUIRE_MESSAGE(success && elem_size_differs, "TEST_VOLUME_VALUE");
    }

    BOOST_AUTO_TEST_CASE(volume_is_not_shared_between_storages) {
        const auto& path = get_file_name("volume_is_not_shared");
        bool success = false;
        {
            StorageT s1;
            auto v1 = s1.open_volume(path, order);
            v1.set(key, value);
            {
                StorageT s2;
                try {
                    s2.open_volume(path, order);
                } catch (std::logic_error&) {
                    success = true;
                }
            }
        }
        StorageT s3;
        auto volume = s3.open_volume(path, order);
        success &= (volume.get(key) == value);

        BOOST_REQUIRE_MESSAGE(success, "TEST_VOLUME_IS_NOT_SHARED_BETWEEN_STORAGES");
    }

    BOOST_AUTO_TEST_SUITE_END()
}
#endif // UNIT_TESTS