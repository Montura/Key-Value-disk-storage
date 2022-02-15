#pragma once

#ifdef UNIT_TESTS

#include "io/block_manager.h"

namespace tests::block_manager_test {
    constexpr int block_size_arr[] = { 128, 256, 512, 1024, 2048, 4096, 8192, 16384 };
    constexpr int total_blocks[] = { 500, 1000};

    namespace details {
        bool test(const int block_size, const int total_blocks) {
            const int total_operations = block_size * total_blocks;
            btree::BlockManager block_manager {block_size, total_blocks};
            tests::ThreadPool tp {10};
            tp.post([&block_manager, total_operations]() {
                for (int i = 0; i < total_operations; ++i) {
                    block_manager.on_new_pos(i);
                }
            });
            tp.join();

            bool success = true;
            for (const auto& block: block_manager.blocks()) {
                uint64_t i = block->usage_count.load();
                success &= (i == block_size);
            }
            uint64_t bucket_count = block_manager.bucket_count();
            success &= (bucket_count == total_blocks);

            uint64_t rebuild_count = block_manager.rebuild_count();
            success &= (bucket_count - (block_manager.total_heap_size) == rebuild_count);

            const auto& lock_ops = block_manager.total_lock_ops();
            success &= (lock_ops == total_blocks);

            const auto& lock_free_ops = block_manager.total_lock_free_ops();
//            success &= (lock_free_ops == total_operations - lock_ops);
//            std::cout << "total lock operations: " << lock_ops << std::endl;
//            std::cout << "total lock free operations: " << lock_free_ops << std::endl;
            std::cout << "lock percent: " << double(lock_ops) / lock_free_ops * 100 << std::endl;
            return success;
        }
    }

    bool run_test(const int block_size) {
        bool success = true;
        for (auto blocks_count : total_blocks) {
            success &= details::test(block_size, blocks_count);
        }
        return success;
    }
}
#endif