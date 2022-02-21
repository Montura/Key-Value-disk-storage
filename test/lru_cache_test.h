#pragma once

#ifdef UNIT_TESTS

#include "io/lru_cache.h"

namespace tests::LRU_test {
    constexpr int block_size_arr[] = { 128, 256, 512, 1024, 2048, 4096, 8192, 16384 };

    namespace details {
        constexpr int blocks_count[] = { 500, 1000, 3000, 5000, 10000 };

        std::vector<std::pair<int32_t, int32_t>> generate_ranges(int32_t max_n, int32_t pairs_count) {
            std::vector<std::pair<int32_t, int32_t>> pairs;

            auto step = max_n / pairs_count;
            pairs.emplace_back(0, step);

            for (auto i = 1; i < pairs_count; ++i) {
                auto end = pairs[i - 1].second;
                pairs.emplace_back(end, end + step);
            }
            pairs[pairs_count - 1].second = max_n;

//            for (auto& pair: pairs)
//                std::cout << "{" << pair.first << ", " << pair.second << "}" << " ";
//            std::cout << std::endl;
            return pairs;
        }

        bool verify_locks_count(const LRUCache <Block>& lru) {
            const auto unique_block_count = static_cast<int>(lru.total_unique_block_count());
            const auto lock_ops = static_cast<int>(lru.total_lock_ops());
//            const auto& lock_free_ops = lru.total_lock_free_ops();
//            std::cout << "total lock operations: " << lock_ops << std::endl;
//            std::cout << "total lock free operations: " << lock_free_ops << std::endl;
//            std::cout << "lock percent: " << double(lock_ops) / lock_free_ops * 100 << std::endl;
            return lock_ops == unique_block_count;
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

        bool test_lru_cache_with_a_single_working_block(const int32_t block_size) {
            const int unique_blocks_count = 1;
            auto verify = [block_size, unique_blocks_count](const int total_ops, auto& lru) -> bool {
                const auto& working_set = lru.working_set();
                bool success = working_set.size() == unique_blocks_count;

                auto reminder = static_cast<int32_t>(total_ops % block_size);
                const auto block_usage_count = static_cast<int32_t>(working_set[0]->usage_count.load());
                success &= (block_usage_count == (reminder > 0 ? reminder : (total_ops > block_size) ? block_size : total_ops));
                success &= verify_lru_state(block_size, 1, lru, total_ops);
                success &= verify_locks_count(lru);
                return success;
            };

            const uint32_t total_ops = 100000000; // 10^8
            bool success = true;
            for (uint32_t ops_count = 1; ops_count < total_ops; ops_count *= 100) {
                btree::LRUCache<Block> lru { block_size, unique_blocks_count };
                run_in_pool_and_join(
                        [&lru, ops_count]() {
                            for (uint32_t i = 0; i < ops_count; ++i) {
                                lru.on_new_pos(i);
                            }
                        });
                success &= verify(ops_count, lru);
            }

            return success;
        }

        bool test_fixed_operation_count_per_block(const int32_t block_size, const int32_t unique_blocks_count) {
            auto verify = [block_size, unique_blocks_count](const int total_ops, auto& lru) -> bool {
                bool success = true;
                auto reminder = static_cast<int32_t>(total_ops % block_size);
                auto working_set = lru.working_set();
                for (auto it = working_set.begin(); it < working_set.end(); ++it) {
                    const auto block_usage_count = static_cast<int32_t>((*it)->usage_count.load());
                    if (std::next(it) == working_set.end()) {
                        success &= (block_usage_count == (reminder > 0 ? reminder : (total_ops > block_size) ? block_size : total_ops));
                    } else {
                        success &= (block_usage_count == block_size);
                    }
                }
                success &= verify_lru_state(block_size, unique_blocks_count / 2, lru, total_ops);
                success &= verify_locks_count(lru);
                return success;
            };

            const uint32_t total_ops = 100000000; // 10^8
            bool success = true;
            for (uint32_t ops_count = 1; ops_count < total_ops; ops_count *= 100) {
                btree::LRUCache<Block> lru{ block_size, unique_blocks_count };
                run_in_pool_and_join(
                        [&lru, ops_count]() {
                            for (uint32_t i = 0; i < ops_count; ++i) {
                                lru.on_new_pos(i);
                            }
                        });
                success &= verify(ops_count, lru);
            }

            return success;
        }

        bool test_lru_cache_for_range(const int32_t block_size, const int32_t unique_blocks_count,
                const std::vector<std::pair<int32_t, int32_t>>& ranges)
        {
            bool success = true;
            btree::LRUCache<Block> lru { block_size, unique_blocks_count };
            for (const auto& range : ranges) {
                int32_t start = range.first;
                int32_t end = range.second;
                run_in_pool_and_join(
                        [&lru, start, end]() {
                            for (auto i = start; i < end; ++i) {
                                lru.on_new_pos(i);
                            }
                        });
                success &= verify_lru_state(block_size, unique_blocks_count / 2, lru, end);
            }
            return success;
        }

        bool test_random_operation_count_per_block(const int32_t block_size, const int32_t total_blocks) {
            const int32_t total_operations = block_size * total_blocks;
            btree::LRUCache<Block> lru { block_size, total_blocks};

            std::vector<std::vector<int32_t>> op_map(total_blocks);
            std::vector<int32_t> usage_load_expected;

            // Generate random count operations over blocks
            for (auto i = 0; i < total_blocks; ++i) {
                op_map[i].resize(i + 1);
                usage_load_expected.push_back(static_cast<int>(op_map[i].size()));
            }
            for (auto i = 0; i < total_blocks; ++i) {
                auto& v = op_map[i];
                auto initial_value = i * block_size;
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

            std::set<int32_t> working_set_usage;
            for (const auto& block: lru.working_set()) {
                working_set_usage.insert(block->usage_count.load());
            }
            bool success = (working_set_usage.size() == op_map.size() / 2);
            success &= std::includes(working_set_usage.begin(), working_set_usage.end(),
                    usage_load_expected.begin() + working_set_usage.size(), usage_load_expected.end());


            success &= verify_lru_state(block_size, total_blocks / 2, lru, total_operations);
            success &= verify_locks_count(lru);
            return success;
        }
    }

    bool test_lru_single_block(const int32_t block_size) {
        return details::test_lru_cache_with_a_single_working_block(block_size);
    }

    bool test_lru_state_for_ranges(const int32_t block_size) {
        bool success = true;
        for (int range_count = 2; range_count < 10; ++range_count) { // at least 2 ranges [0, n/2], (n/2, n);
            const int32_t MB = 1000000; // 10^6
            std::vector<std::pair<int32_t, int32_t>> ranges = details::generate_ranges(MB, range_count);
            for (auto unique_blocks_count: details::blocks_count) {
                success &= details::test_lru_cache_for_range(block_size, unique_blocks_count,ranges);
            }
        }
        return success;
    }

    bool test_fixed_operations_count(const int32_t block_size) {
        bool success = true;
        for (auto unique_blocks_count : details::blocks_count) {
            success &= details::test_fixed_operation_count_per_block(block_size, unique_blocks_count);
        }
        return success;
    }

    bool test_random_operations_count(const int32_t block_size) {
        bool success = true;
        for (auto unique_blocks_count : details::blocks_count) {
            success &= details::test_random_operation_count_per_block(block_size, unique_blocks_count);
        }
        return success;
    }
}
#endif