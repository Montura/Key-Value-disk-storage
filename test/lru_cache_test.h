#pragma once

#ifdef UNIT_TESTS

#include "io/lru_cache.h"

namespace tests::block_manager_test {
    constexpr int block_size_arr[] = { 128, 256, 512, 1024, 2048, 4096, 8192, 16384 };

    namespace details {
        constexpr int blocks_count[] = { 500, 1000, 3000, 5000, 10000 };

        std::vector<std::pair<int, int>> generate_ranges(uint32_t max_n, int pairs_count) {
            std::vector<std::pair<int, int>> pairs;

            int step = max_n / pairs_count;
            pairs.emplace_back(0, step);

            for (int i = 1; i < pairs_count; ++i) {
                auto end = pairs[i - 1].second;
                pairs.emplace_back(end, end + step);
            }
            pairs[pairs_count - 1].second = max_n;

//            for (auto& pair: pairs)
//                std::cout << "{" << pair.first << ", " << pair.second << "}" << " ";
//            std::cout << std::endl;
            return pairs;
        }

        bool verify_locks_count(const int total_blocks, const LRUCache <Block>& lru) {
            const auto lock_ops =static_cast<int>(lru.total_lock_ops());
            const auto& lock_free_ops = lru.total_lock_free_ops();
//            success &= (lock_free_ops == total_operations - lock_ops);
//            std::cout << "total lock operations: " << lock_ops << std::endl;
//            std::cout << "total lock free operations: " << lock_free_ops << std::endl;
            std::cout << "lock percent: " << double(lock_ops) / lock_free_ops * 100 << std::endl;
            return lock_ops == total_blocks;
        }

        bool verify_lru_state(const int block_size,
                const int expected_cache_size,
                const LRUCache <Block>& lru,
                const int max_address) {
            const auto cache_size = static_cast<int>(lru.cache_size());
            const auto unique_blocks_count = static_cast<int>(lru.total_unique_block_count());
            const auto cache_rebuild_count = static_cast<int>(lru.total_cache_rebuild_count());

            bool success = (cache_size == expected_cache_size);
            success &= (unique_blocks_count == (max_address / block_size + (max_address % block_size != 0)));
            success &= (unique_blocks_count > cache_size)
                    ? (unique_blocks_count - cache_size) == cache_rebuild_count
                    : (cache_rebuild_count == 0);
            return success;
        }

        template <typename Func>
        void run_in_pool_and_join(Func && f) {
            tests::ThreadPool tp {10};
            tp.post(f);
            tp.join();
        }

        bool test_fixed_operation_count_per_block(const int block_size, const int unique_blocks_count) {
            const int total_operations = block_size * unique_blocks_count;
            btree::LRUCache<Block> lru { block_size, unique_blocks_count };
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
            success &= verify_lru_state(block_size, unique_blocks_count / 2, lru, total_operations);
            success &= verify_locks_count(unique_blocks_count, lru);

            return success;
        }

        bool test_lru_cache_for_range(const int block_size, const int unique_blocks_count,
                const std::vector<std::pair<int, int>>& ranges)
        {
            bool success = true;
            btree::LRUCache<Block> lru { block_size, unique_blocks_count };
            for (const auto& range : ranges) {
                int start = range.first;
                int end = range.second;
                run_in_pool_and_join(
                        [&lru, start, end]() {
                            for (int i = start; i < end; ++i) {
                                lru.on_new_pos(i);
                            }
                        });
                success &= verify_lru_state(block_size, unique_blocks_count / 2, lru, end);
            }
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


            success &= verify_lru_state(block_size, total_blocks / 2, lru, total_operations);
            success &= verify_locks_count(total_blocks, lru);
            return success;
        }
    }

    bool test_lru_state_for_ranges(const int block_size) {
        bool success = true;
        for (int range_count = 2; range_count < 10; ++range_count) { // at least 2 ranges [0, n/2], (n/2, n);
            const uint32_t MB = 1000000; // 10^8
            std::vector<std::pair<int, int>> ranges = details::generate_ranges(MB, range_count);
            for (auto unique_blocks_count: details::blocks_count) {
                success &= details::test_lru_cache_for_range(block_size, unique_blocks_count,ranges);
            }
        }
        return success;
    }

    bool test_fixed_operations_count(const int block_size) {
        bool success = true;
        for (auto unique_blocks_count : details::blocks_count) {
            success &= details::test_fixed_operation_count_per_block(block_size, unique_blocks_count);
        }
        return success;
    }

    bool test_random_operations_count(const int block_size) {
        bool success = true;
        for (auto unique_blocks_count : details::blocks_count) {
            success &= details::test_random_operation_count_per_block(block_size, unique_blocks_count);
        }
        return success;
    }
}
#endif