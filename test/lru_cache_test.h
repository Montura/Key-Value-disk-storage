#pragma once

#ifdef UNIT_TESTS

#include "io/lru_cache.h"

namespace tests::LRU_test {
    constexpr int32_t block_size_arr_len = 8;
    constexpr int32_t block_size_arr[block_size_arr_len] = { 128, 256, 512, 1024, 2048, 4096, 8192, 16384 };

    namespace details {
        class TestBlock {
            std::atomic<int64_t> m_usage_count = 0;
            bip::mapped_region mapped_region;
        public:
            const int32_t m_size;
            const int64_t mapped_offset;

            TestBlock(const std::string& path, int64_t file_offset, const int32_t size = 4096, bip::mode_t mapping_mode = bip::read_write) :
                    m_size(size), mapped_offset(file_offset)
            {
                auto file_mapping = bip::file_mapping(path.data(), mapping_mode);
                mapped_region = bip::mapped_region(file_mapping, mapping_mode, mapped_offset, size);
            };

            const std::atomic<int64_t>& usage_count() {
                return m_usage_count;
            }

            void add_ref() {
                ++m_usage_count;
            }
        };


        template <typename T>
        using VectorT = std::vector<T>;

        constexpr int blocks_count[] = { 500, 1000, 3000 } ;// 5000, 10000 };

        constexpr int32_t FILE_50MB = 16384 * 3000;
        const char* path = "../lru_block_mapped_test.txt";

        const char* init_file() {
            bool file_exists = fs::exists(path);
            if (!file_exists) {
                btree::file::create_file(path, FILE_50MB);
            }
            return path;
        }

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
                if (block_size < std::distance(begin, end)) {
                    std::fill(begin + block_size, end, initial_value);
                }
            };
            for (auto i = 0; i < unique_blocks_count; ++i) {
                VectorT<int32_t>& v = address_vector_map[i];
                resize_and_fill_vector(v, i);
                total_operations += block_size;
            }
            return total_operations;
        }

        template <typename Block>
        bool verify_locks_count(const LRUCache <Block>& lru) {
            const auto unique_block_count = static_cast<int>(lru.total_unique_block_count());
            const auto lock_ops = static_cast<int>(lru.total_lock_ops());
//            const auto& lock_free_ops = lru.total_lock_free_ops();
//            std::cout << "total lock operations: " << lock_ops << std::endl;
//            std::cout << "total lock free operations: " << lock_free_ops << std::endl;
//            std::cout << "lock percent: " << double(lock_ops) / lock_free_ops * 100 << std::endl;
            return lock_ops == unique_block_count;
        }

        template <typename Block>
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
            const auto block_usage_count = static_cast<int32_t>(block->usage_count().load());
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

        template <typename Block>
        bool test_lru_cache_with_a_single_working_block(const int32_t block_size) {
            const int32_t cache_size = 1;

            auto verify = [block_size, cache_size](const int32_t total_ops, auto& lru) -> bool {
                const int unique_blocks_count = 1;
                const auto& working_set = lru.working_set();
                bool success = working_set.size() == unique_blocks_count;
                success &= verify_lru_block_usage(total_ops, block_size, working_set[0], true);
                success &= verify_lru_state(block_size, cache_size, lru, total_ops);
                success &= verify_locks_count(lru);
                return success;
            };

            const uint32_t total_ops = 100000000; // 10^8
            bool success = true;
            for (uint32_t ops_count = 1; ops_count < total_ops; ops_count *= 100) {
                btree::LRUCache<Block> lru{ cache_size, init_file() };
                run_in_pool_and_join(
                        [&lru, ops_count, block_size]() {
                            for (uint32_t i = 0; i < ops_count; ++i) {
                                lru.on_new_pos(i, sizeof(i), block_size);
                            }
                        });
                success &= verify(ops_count, lru);
            }

            return success;
        }

        template <typename Block>
        bool test_fixed_operation_count_per_block(const int32_t block_size, const int32_t unique_blocks_count) {
            const int32_t cache_size = unique_blocks_count / 2;

            auto verify = [block_size, cache_size](const int32_t total_ops, auto& lru) -> bool {
                bool success = true;
                auto working_set = lru.working_set();
                for (auto it = working_set.begin(); it < working_set.end(); ++it) {
                    bool is_last_block = std::next(it) == working_set.end();
                    verify_lru_block_usage(total_ops, block_size, *it, is_last_block);
                }
                success &= verify_lru_state(block_size, cache_size, lru, total_ops);
                success &= verify_locks_count(lru);
                return success;
            };

            const uint32_t total_ops = 100000000; // 10^8
            bool success = true;
            for (uint32_t ops_count = 1; ops_count < total_ops; ops_count *= 100) {
                btree::LRUCache<Block> lru{ cache_size, init_file() };
                run_in_pool_and_join(
                        [&lru, ops_count, block_size]() {
                            for (uint32_t i = 0; i < ops_count; ++i) {
                                lru.on_new_pos(i, sizeof(i), block_size);
                            }
                        });
                success &= verify(ops_count, lru);
            }

            return success;
        }

        template <typename Block>
        bool test_lru_cache_for_range(const int32_t block_size, const int32_t unique_blocks_count,
                const VectorT<std::pair<int32_t, int32_t>>& ranges)
        {
            bool success = true;
            const int32_t cache_size = unique_blocks_count / 2;
            btree::LRUCache<Block> lru{ cache_size, init_file() };
            for (const auto& range : ranges) {
                int32_t start = range.first;
                int32_t end = range.second;
                run_in_pool_and_join(
                        [&lru, start, end, block_size]() {
                            for (auto i = start; i < end; ++i) {
                                lru.on_new_pos(i, sizeof(i), block_size);
                            }
                        });
                success &= verify_lru_state(block_size, cache_size, lru, end);
            }
            return success;
        }

        template <typename Block>
        bool test_random_operation_count_per_block(const int32_t block_size, const int32_t unique_blocks_count) {
            VectorT<VectorT<int32_t>> address_vector_map(unique_blocks_count);
            const auto total_operations = generate_test_block_operations(address_vector_map, block_size, unique_blocks_count);
            const int32_t cache_size = unique_blocks_count / 2;

            VectorT<int32_t> top_usages_in_working_set(cache_size);
            for (size_t i = 0; i < top_usages_in_working_set.size(); ++i) {
                top_usages_in_working_set[i] = static_cast<int32_t>(address_vector_map[i + cache_size].size());
            }

            btree::LRUCache<Block> lru { cache_size, init_file() };
            run_in_pool_and_join(
                    [&lru, &address_vector_map, block_size]() {
                        for (const auto& address_vector: address_vector_map) {
                            for (const auto& address: address_vector) {
                                lru.on_new_pos(address, sizeof(address), block_size);
                            }
                        }
                    });

            std::set<int64_t> working_set_usage;
            for (const auto& block: lru.working_set()) {
                working_set_usage.insert(block->usage_count().load());
            }
            bool success = (working_set_usage.size() == top_usages_in_working_set.size());
            success &= std::includes(working_set_usage.begin(), working_set_usage.end(),
                    top_usages_in_working_set.begin(), top_usages_in_working_set.end());

            success &= verify_lru_state(block_size, cache_size, lru, total_operations);
            success &= verify_locks_count(lru);
            return success;
        }

        template <typename Block>
        bool test_lru_random_block_size(const int32_t unique_blocks_count) {
            const int32_t cache_size = unique_blocks_count / 2;
            btree::LRUCache<Block> lru { cache_size, init_file() };

            auto verify = [](const uint32_t total_ops, auto& lru,
                    const int32_t block_idx, const int32_t block_size) -> bool
                {
                    const auto& working_set = lru.working_set();
                    bool success = static_cast<int32_t>(working_set.size()) == (block_idx + 1);
                    success &= verify_lru_block_usage(total_ops, block_size, working_set[block_idx], false);
                    success &= (lru.total_cache_rebuild_count() == 0);
                    success &= verify_locks_count(lru);
                    return success;
                };

            bool success = true;
            int32_t shift = 0;
            for (int32_t block_idx = 0; block_idx < block_size_arr_len; ++block_idx) {
                const auto block_size = block_size_arr[block_idx];
                const auto total_ops = block_size;
                run_in_pool_and_join(
                        [&lru, total_ops, shift, block_size]() {
                            for (int32_t i = 0; i < total_ops; ++i) {
                                lru.on_new_pos(shift + i, sizeof(i), block_size);
                            }
                        });
                shift += block_size;
                success &= verify(total_ops, lru, block_idx, block_size);
            }
            return success;
        }

        template <typename Block>
        bool test_lru_single_block(const int32_t block_size) {
            return test_lru_cache_with_a_single_working_block<Block>(block_size);
        }

        template <typename Block>
        bool test_lru_state_for_ranges(const int32_t block_size) {
            bool success = true;
            for (int range_count = 2; range_count < 10; ++range_count) { // at least 2 ranges [0, n/2], (n/2, n);
                const int32_t MB = 1000000; // 10^6
                VectorT<std::pair<int32_t, int32_t>> ranges = generate_ranges(MB, range_count);
                for (auto unique_blocks_count: blocks_count) {
                    success &= test_lru_cache_for_range<Block>(block_size, unique_blocks_count,ranges);
                }
            }
            return success;
        }

        template <typename Block>
        bool test_fixed_operations_count(const int32_t block_size) {
            bool success = true;
            for (auto unique_blocks_count : blocks_count) {
                success &= test_fixed_operation_count_per_block<Block>(block_size, unique_blocks_count);
            }
            return success;
        }

        template <typename Block>
        bool test_random_operations_count(const int32_t block_size) {
            bool success = true;
            for (auto unique_blocks_count : details::blocks_count) {
                success &= test_random_operation_count_per_block<Block>(block_size, unique_blocks_count);
            }
            return success;
        }

        template <typename Block>
        bool test_lru_random_block_size() {
            bool success = true;
            for (auto unique_blocks_count : details::blocks_count) {
                success &= test_lru_random_block_size<Block>(unique_blocks_count);
            }
            return success;
        }
    }

    bool run_test_lru_single_block(const int32_t block_size) {
        return  details::test_lru_single_block<details::TestBlock>(block_size) &&
                details::test_lru_single_block<MappedRegionBlock>(block_size);
    }

    bool run_test_lru_state_for_ranges(const int32_t block_size) {
        return  details::test_lru_state_for_ranges<details::TestBlock>(block_size) &&
                details::test_lru_state_for_ranges<MappedRegionBlock>(block_size);
    }

    bool run_test_fixed_operations_count(const int32_t block_size) {
        return  details::test_fixed_operations_count<details::TestBlock>(block_size) &&
                details::test_fixed_operations_count<MappedRegionBlock>(block_size);
    }

    bool run_test_random_operations_count(const int32_t block_size) {
        return  details::test_random_operations_count<details::TestBlock>(block_size) &&
                details::test_random_operations_count<MappedRegionBlock>(block_size);
    }

    bool run_test_lru_random_block_size() {
        return  details::test_lru_random_block_size<details::TestBlock>() &&
                details::test_lru_random_block_size<MappedRegionBlock>();
    }
}
#endif