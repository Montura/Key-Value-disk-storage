#pragma once

#ifdef UNIT_TESTS

#include "io/lru_cache.h"

namespace tests::block_manager_test {
    constexpr int block_size_arr[] = { 16, 128, 256, 512, 1024, 2048, 4096, 8192, 16384 };

    namespace details {
        bool verify_locks_count(const int total_blocks, const LRUCache <Block>& lru) {
            const auto lock_ops =static_cast<int>(lru.total_lock_ops());
            const auto& lock_free_ops = lru.total_lock_free_ops();
//            success &= (lock_free_ops == total_operations - lock_ops);
//            std::cout << "total lock operations: " << lock_ops << std::endl;
//            std::cout << "total lock free operations: " << lock_free_ops << std::endl;
            std::cout << "lock percent: " << double(lock_ops) / lock_free_ops * 100 << std::endl;
            return lock_ops == total_blocks;
        }

        bool verify_working_set_size_and_locks(const int total_blocks, const LRUCache<Block>& lru) {
            const auto total_block_count = static_cast<int>(lru.total_block_count());
            return (total_block_count == total_blocks) &&
                   (total_block_count - lru.working_set_size) == lru.total_working_set_rebuild_count();
        }

        template <typename Func>
        void run_in_pool_and_join(Func && f) {
            tests::ThreadPool tp {10};
            tp.post(f);
            tp.join();
        }

        bool test_fixed_operation_count_per_block(const int block_size, const int total_blocks) {
            const int total_operations = block_size * total_blocks;
            btree::LRUCache<Block> lru { block_size, total_blocks };
            run_in_pool_and_join(
                    [&lru, total_operations]() {
                        for (int i = 0; i < total_operations; ++i) {
                            lru.on_new_pos(i);
                        }
                    });

            bool success = true;
            for (const auto& block: lru.working_set()) {
                const auto i = static_cast<int>(block->usage_count.load());
                success &= (i == block_size);
            }
            success &= verify_working_set_size_and_locks(total_blocks, lru);
            success &= verify_locks_count(total_blocks, lru);

            return success;
        }


        bool test_random_operation_count_per_block(const int block_size, const int total_blocks) {
            const int total_operations = block_size * total_blocks;
            btree::LRUCache<Block> lru { block_size, total_blocks};

            std::vector<std::vector<int>> op_map(total_blocks);
            std::vector<int> usage_load_expected;

            // Generate random count operations over blocks
            for (int i = 0; i < total_blocks; ++i) {
                op_map[i].resize(i + 1);
                usage_load_expected.push_back(static_cast<int>(op_map[i].size()));
            }
            for (int i = 0; i < total_blocks; ++i) {
                auto& v = op_map[i];
                int initial_value = i * block_size;
                std::iota(v.begin(), v.end(), initial_value);
                if (v.begin() + block_size < v.end()) {
                    std::fill(v.begin() + block_size, v.end(), initial_value);
                }
            }

            run_in_pool_and_join(
                    [&lru, &op_map]() {
                        for (const auto& e: op_map) {
                            for (const auto& v: e) {
                                lru.on_new_pos(v);
                            }
                        }
                    });

            std::set<int> working_set_usage;
            for (const auto& block: lru.working_set()) {
                working_set_usage.insert(block->usage_count.load());
            }
            bool success = (working_set_usage.size() == op_map.size() / 2);
            success &= std::includes(working_set_usage.begin(), working_set_usage.end(),
                    usage_load_expected.begin() + working_set_usage.size(), usage_load_expected.end());


            const auto bucket_count = static_cast<int>(lru.total_block_count());
            success &= verify_working_set_size_and_locks(total_blocks, lru);
            success &= verify_locks_count(total_blocks, lru);
            return success;
        }
    }

    bool run_test(const int block_size) {
        bool success = true;
        constexpr int total_blocks[] = { 500, 1000, 3000, 5000, 10000 };
        for (auto blocks_count : total_blocks) {
            success &= details::test_fixed_operation_count_per_block(block_size, blocks_count);
        }
        return success;
    }

    bool run_test_2(const int block_size) {
        bool success = true;
        constexpr int total_blocks[] = { 10, 500, 1000, 3000, 5000, 10000 };
        for (auto blocks_count : total_blocks) {
            success &= details::test_random_operation_count_per_block(block_size, blocks_count);
        }
        return success;
    }
}
#endif