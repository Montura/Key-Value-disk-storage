#pragma once

#ifdef UNIT_TESTS

#include "io/lru_cache.h"
#include "io/mapped_region.h"


namespace tests::LRU_mapped_file_test {

    namespace details {
        const int32_t KB = 4096;
        const int32_t CACHE_SIZE = 250;
        const char* path = "../lru_mapped_test.txt";
    }

    bool test_region() {
        const auto& path = details::path;

        bool file_exists = fs::exists(path);
        if (!file_exists) {
            file::create_file(path, details::KB);
        }
        LRUCache<MappedRegionBlock> lru(details::CACHE_SIZE, path);


//        lru.on_new_pos(1);
        return true;
    }

}

#endif