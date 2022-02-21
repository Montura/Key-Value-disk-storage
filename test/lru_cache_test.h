#pragma once

#ifdef UNIT_TESTS

#include "io/lru_cache.h"

namespace tests::LRU_test {
    constexpr int block_size_arr[] = { 128, 256, 512, 1024, 2048, 4096, 8192, 16384 };

    namespace details {
        template <typename T>
        using VectorT = std::vector<T>;

        constexpr int blocks_count[] = { 500, 1000, 3000, 5000, 10000 };

        VectorT<std::pair<int32_t, int32_t>> generate_ranges(int32_t max_n, int32_t pairs_count) {
            VectorT<std::pair<int32_t, int32_t>> pairs;

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

        int32_t generate_test_block_operations(VectorT<VectorT<int32_t>>& address_vector_map, const int32_t block_size,
                const int32_t unique_blocks_count)
        {
            // Generate random count operations over blocks
            //  - Fix the block size = M
            //  - Fix the block count = N

            //  - For each block we generate N + 1 operations to access the address inside the current block, we got:
            //    * unique address accesses count are from [1; M]
            //    * repeated address accesses count are from [0; (N - M)]

            //  block  |  address to be accessed via LRU
            //    0   -> [0]
            //    1   -> [M, M + 1]
            //    2   -> [2 * M, 2 * M + 1, 2 * M + 2]

            //            |                      M elements                   |
            // K(== M) -> [(K + 0) * M, (K + 0) * M + 1, ... , (K + 0) * M + K]
            //            |                      M elements                   |             1 element          |
            //  K + 1  -> [(K + 1) * M, (K + 1) * M + 1, ... , (K + 1) * M + K, (K + 1) * M]
            //            |                      M elements                   |             2 elements         |
            //  K + 2  -> [(K + 2) * M, (K + 2) * M + 1, ... , (K + 2) * M + K, (K + 2) * M, (K + 2) * M]
            //            |                       M elements                  |          (N - M) elements      |
            //    N   ->  [(N + 0) * M, (N + 0) * M + 1, ... , (N + 0 * M) + M,  (N + 0) * M, ... , (N + 0) * M]

            int32_t total_operations = 0;
            auto resize_and_fill_vector = [block_size](auto& v, int n) {
                auto initial_value = n * block_size;
                v.resize(n + 1);
                const auto& begin = v.begin();
                const auto& end = v.end();
                std::iota(begin, end, initial_value);
                const auto& block_end_it = begin + block_size;
                if (block_end_it < end) {
                    std::fill(block_end_it, end, initial_value);
                }
            };
            for (auto i = 0; i < unique_blocks_count; ++i) {
                VectorT<int32_t>& v = address_vector_map[i];
                resize_and_fill_vector(v, i);
                total_operations += block_size;
            }
            return total_operations;
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

        template <typename Block>
        bool verify_lru_block_usage(const int32_t total_ops, const int32_t block_size,
                const std::shared_ptr<Block>& block, bool is_last_block)
        {
            const auto block_usage_count = static_cast<int32_t>(block->usage_count.load());
            auto reminder = static_cast<int32_t>(total_ops % block_size);
            return is_last_block
                ? (block_usage_count == (reminder > 0 ? reminder : (total_ops > block_size) ? block_size : total_ops))
                : (block_usage_count == block_size);
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
                success &= verify_lru_block_usage(total_ops, block_size, working_set[0], true);
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
            auto verify = [block_size, unique_blocks_count](const int32_t total_ops, auto& lru) -> bool {
                bool success = true;
                auto working_set = lru.working_set();
                for (auto it = working_set.begin(); it < working_set.end(); ++it) {
                    bool is_last_block = std::next(it) == working_set.end();
                    verify_lru_block_usage(total_ops, block_size, *it, is_last_block);
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
                const VectorT<std::pair<int32_t, int32_t>>& ranges)
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

        bool test_random_operation_count_per_block(const int32_t block_size, const int32_t unique_blocks_count) {
            VectorT<VectorT<int32_t>> address_vector_map(unique_blocks_count);
            const auto total_operations = generate_test_block_operations(address_vector_map, block_size, unique_blocks_count);

            VectorT<int32_t> top_usages_in_working_set(unique_blocks_count / 2);
            const size_t shift = unique_blocks_count / 2;
            for (auto i = 0; i < top_usages_in_working_set.size(); ++i) {
                top_usages_in_working_set[i] = static_cast<int32_t>(address_vector_map[i + shift].size());
            }

            btree::LRUCache<Block> lru { block_size, unique_blocks_count};
            run_in_pool_and_join(
                    [&lru, &address_vector_map]() {
                        for (const auto& address_vector: address_vector_map) {
                            for (const auto& address: address_vector) {
                                lru.on_new_pos(address);
                            }
                        }
                    });

            std::set<int32_t> working_set_usage;
            for (const auto& block: lru.working_set()) {
                working_set_usage.insert(block->usage_count.load());
            }
            bool success = (working_set_usage.size() == top_usages_in_working_set.size());
            success &= std::includes(working_set_usage.begin(), working_set_usage.end(),
                    top_usages_in_working_set.begin(), top_usages_in_working_set.end());

            success &= verify_lru_state(block_size, unique_blocks_count / 2, lru, total_operations);
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
            details::VectorT<std::pair<int32_t, int32_t>> ranges = details::generate_ranges(MB, range_count);
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