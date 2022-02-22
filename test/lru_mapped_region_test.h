#pragma once

#ifdef UNIT_TESTS

#include "io/lru_cache.h"
#include "io/mapped_region.h"


namespace tests::LRU_mapped_file_test {

    namespace details {

    }

    bool test_region() {
        LRUCache<MappedRegionBlock> lru(4096, 1);

        lru.on_new_pos(1);
        return true;
    }

}

#endif