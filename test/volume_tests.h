#pragma once

#ifdef UNIT_TESTS

#include "storage.h"

namespace tests {
    using StorageT = btree::Storage<int, int>;

    namespace {
        std::string path = "../test_storage.txt";
        const int order = 2;
        const int key = 0;
        const int value = 123456789;
    }

    BOOST_AUTO_TEST_SUITE(volume_tests)

    BOOST_AUTO_TEST_CASE(olue_open_close) {
        StorageT s1;
        auto v1 = s1.open_volume(path, order);
        v1.set(key, value);
        s1.close_volume(v1);

        StorageT s2;
        auto v2 = s2.open_volume(path, order);
        bool success = (v2.get(key) == value);

        BOOST_REQUIRE_MESSAGE(success, "TEST_VOLUME_OPEN_CLOSE");
    }

    BOOST_AUTO_TEST_CASE(volume_is_not_shared_between_storages) {
        bool success = false;
        {
            StorageT s1;
            auto v1 = s1.open_volume(path, order);
            v1.set(key, value);
            {
                StorageT s2;
                try {
                    s2.open_volume(path, order);
                } catch (std::logic_error& e) {
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