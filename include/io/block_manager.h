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
        bool operator<(const Block& right) const {
            return usage_count < (right.usage_count);
        }
    };

    class BlockManager {
        using HashTIt = typename std::map<const uint64_t, std::shared_ptr<Block>>::iterator;

        std::vector<std::shared_ptr<Block>> heap;
        size_t heap_end_pos = 0;
        std::map<uint64_t, std::shared_ptr<Block>> hash_table;
        std::mutex mutex;

        std::atomic<uint64_t> m_bucket_count = 0;
        std::atomic<uint64_t> m_rebuild_count = 0;
        std::atomic<uint64_t> m_total_lock_ops = 0;
        std::atomic<uint64_t> m_total_lock_free_ops = 0;
    public:
        const uint64_t total_heap_size;
        const uint64_t block_size;

        BlockManager(int64_t block_size, int64_t block_count) : block_size(block_size), total_heap_size(block_count / 2) {
            heap.resize(total_heap_size);
        }

        HashTIt on_new_pos(const uint64_t pos) {
            uint64_t bucket_idx = round_pos(pos);
            const auto& find_it = hash_table.find(bucket_idx);
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

        const std::vector<std::shared_ptr<Block>>& blocks() const {
            return heap;
        }

        uint64_t bucket_count() const {
            return m_bucket_count.load();
        }

        uint64_t rebuild_count() const {
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
            const auto&[emplace_it, success] = hash_table.try_emplace(bucket_idx, new Block(pos));
            if (success) {
                if (heap_end_pos == total_heap_size)
                    remove_the_least_used_block();
                heap[heap_end_pos++] = emplace_it->second;
                std::make_heap(heap.begin(), heap.end());
                ++m_bucket_count;
            }
            return emplace_it;
        }

        void remove_the_least_used_block() {
            std::pop_heap(heap.begin(), heap.end());
            auto value_to_remove = heap.back();
            uint64_t k = round_pos(value_to_remove->addr);
            auto removed_count = hash_table.erase(k);
            assert(removed_count == 1);
            heap[--heap_end_pos].reset();
            ++m_rebuild_count;
        }

        uint64_t round_pos(uint64_t addr) const {
            return (addr / block_size) * block_size;
        }
    };
}