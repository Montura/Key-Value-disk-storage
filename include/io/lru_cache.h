#pragma once

#include "utils/thread_pool.h"
#include <unordered_map>
#include <algorithm>
#include <atomic>

namespace btree {
    struct Block {
        uint64_t addr = 0;
        std::atomic<uint64_t> usage_count = 0;

        explicit Block(uint64_t addr) : addr(addr) {};
    };

    struct Comparator {
        bool operator()(const std::shared_ptr<Block>& lhs, const std::shared_ptr<Block>& rhs) {
            return lhs->usage_count > rhs->usage_count;
        };
    };

    template <typename T>
    class LRUCache {
        using HashTIt = typename std::unordered_map<uint64_t, std::shared_ptr<T>>::iterator;

        std::vector<std::shared_ptr<T>> min_heap;
        size_t heap_end_pos = 0;
        const Comparator min_heap_comparator;

        std::unordered_map<uint64_t, std::shared_ptr<T>> hash_table;
        std::mutex mutex;

        std::atomic<uint64_t> m_bucket_count = 0;
        std::atomic<uint64_t> m_rebuild_count = 0;
        std::atomic<uint64_t> m_total_lock_ops = 0;
        std::atomic<uint64_t> m_total_lock_free_ops = 0;
    public:
        const uint64_t block_size;
        const uint64_t working_set_size;

        LRUCache(int64_t block_size, int64_t block_count) : block_size(block_size), working_set_size(block_count / 2) {
            min_heap.resize(working_set_size);
        }

        HashTIt on_new_pos(const uint64_t pos) {
            uint64_t bucket_idx = round_pos(pos);
            const auto find_it = hash_table.find(bucket_idx);
            if (find_it == hash_table.end()) {
                std::unique_lock lock(mutex);
                auto emplace_it = add_new_block(pos, bucket_idx);
                m_total_lock_ops++;
                emplace_it->second->usage_count++;
                return emplace_it;
            } else {
                m_total_lock_free_ops++;
                find_it->second->usage_count++;
                return find_it;
            }
        }

        const std::vector<std::shared_ptr<Block>>& working_set() const {
            return min_heap;
        }

        uint64_t total_block_count() const {
            return m_bucket_count.load();
        }

        uint64_t total_working_set_rebuild_count() const {
            return m_rebuild_count.load();
        }

        uint64_t total_lock_ops() const {
            return m_total_lock_ops.load();
        }

        uint64_t total_lock_free_ops() const {
            return m_total_lock_free_ops.load();
        }
    private:
        HashTIt add_new_block(const uint64_t pos, uint64_t bucket_idx) {
            const auto&[emplace_it, success] = hash_table.try_emplace(bucket_idx, new T(pos));
            if (success) {
                if (heap_end_pos == working_set_size)
                    remove_the_least_used_block();
                min_heap[heap_end_pos++] = emplace_it->second;
                ++m_bucket_count;
            }
            return emplace_it;
        }

        void remove_the_least_used_block() {
            std::make_heap(min_heap.begin(), min_heap.end(), min_heap_comparator);
            std::pop_heap(min_heap.begin(), min_heap.end(), min_heap_comparator);
            auto value_to_remove = min_heap.back();
            uint64_t k = round_pos(value_to_remove->addr);
            auto removed_count = hash_table.erase(k);
            assert(removed_count == 1);
            min_heap[--heap_end_pos].reset();
            ++m_rebuild_count;
        }

        uint64_t round_pos(uint64_t addr) const {
            return (addr / block_size) * block_size;
        }
    };
}