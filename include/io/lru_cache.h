#pragma once

#include "utils/thread_pool.h"
#include <unordered_map>
#include <algorithm>
#include <atomic>

namespace btree {
    template <typename BlockT>
    struct Comparator {
        bool operator()(const std::shared_ptr<BlockT>& lhs, const std::shared_ptr<BlockT>& rhs) {
            return lhs->usage_count() > rhs->usage_count();
        };
    };

    // custom hash can be a standalone function object:
    struct Int64Hash {
        std::size_t operator()(const int64_t& value) const noexcept {
            return value;
        }
    };

    template <typename T>
    class LRUCache {
        using HashTIt = typename std::unordered_map<int64_t, std::shared_ptr<T>, Int64Hash>::iterator;

        const std::string path;
        std::vector<std::shared_ptr<T>> min_heap;
        size_t heap_end_pos = 0;
        const Comparator<T> min_heap_comparator;

        std::unordered_map<int64_t, std::shared_ptr<T>, Int64Hash> hash_table;
        std::mutex mutex;

        std::atomic<int64_t> m_unique_block_count = 0;
        std::atomic<int64_t> m_cache_rebuild_count = 0;
        std::atomic<int64_t> m_total_lock_ops = 0;
        std::atomic<int64_t> m_total_lock_free_ops = 0;
        const int64_t m_cache_size;

    public:

        LRUCache(const int64_t cache_size, const std::string& path) :
            path(path),
            m_cache_size(cache_size)
        {
            hash_table.reserve(m_cache_size);
            min_heap.resize(m_cache_size);
        }

        std::pair<int64_t, std::shared_ptr<T>> on_new_pos(const int64_t pos, const int64_t blob_size, const int64_t lru_block_size = 4096) {
            const auto max_block_size = (blob_size > lru_block_size) ? blob_size : lru_block_size;
            const auto begin_it = lru_contains(pos);
            if (begin_it == hash_table.end()) {
                std::unique_lock lock(mutex);
                return add_block(pos, max_block_size);
            } else {
                if (begin_it->second->contains(pos + blob_size)) {
                    m_total_lock_free_ops++;
                    begin_it->second->add_ref();
                    return std::make_pair(pos, begin_it->second);
                } else {
                    std::unique_lock lock(mutex);
                    const auto aligned_pos = align_pos(pos + blob_size, max_block_size);
                    return add_block(aligned_pos, max_block_size);
                }
            }
        }

        std::shared_ptr<T> on_new_pos_a(const int64_t pos, const int64_t lru_block_size = 4096) {
            const int32_t block_size = sizeof(pos);
            const auto max_block_size = (block_size > lru_block_size) ? block_size : lru_block_size;
            const auto find_it = lru_contains(pos);
            if (find_it == hash_table.end()) {
                std::unique_lock lock(mutex);
                auto emplace_it = add_new_block(pos, max_block_size);
                m_total_lock_ops++;
                emplace_it->second->add_ref();
                return emplace_it->second;
            } else {
                m_total_lock_free_ops++;
                find_it->second->add_ref();
                return find_it->second;
            }
        }

        std::vector<std::shared_ptr<T>> working_set() const {
            return std::vector<std::shared_ptr<T>>(min_heap.begin(), min_heap.begin() + heap_end_pos);
        }

        int64_t cache_size() const {
            return min_heap.size();
        }

        int64_t total_unique_block_count() const {
            return m_unique_block_count.load();
        }

        int64_t total_cache_rebuild_count() const {
            return m_cache_rebuild_count.load();
        }

        int64_t total_lock_ops() const {
            return m_total_lock_ops.load();
        }

        int64_t total_lock_free_ops() const {
            return m_total_lock_free_ops.load();
        }

        int64_t align_pos(int64_t addr, const int block_size) const {
            return (addr / block_size) * block_size;
        }

        void clear() {
            min_heap.clear();
            hash_table.clear();
        }
    private:
        std::pair<int64_t, std::shared_ptr<T>> add_block(const int64_t pos, const int64_t max_block_size) {
            auto emplace_it = add_new_block(pos, max_block_size);
            m_total_lock_ops++;
            emplace_it->second->add_ref();
            return std::make_pair(pos, emplace_it->second);
        }

        HashTIt lru_contains(const int64_t pos) {
            return std::find_if(hash_table.begin(), hash_table.end(),
                    [pos](const auto& elem) {
                        return elem.second->contains(pos);
                    }
            );
        }
        
        HashTIt add_new_block(const int64_t pos, const int32_t block_size) {
            const auto&[emplace_it, success] = hash_table.try_emplace(pos, new T(path, pos, block_size));
            if (success) {
                if (heap_end_pos == m_cache_size)
                    remove_the_least_used_block();
                min_heap[heap_end_pos++] = emplace_it->second;
                ++m_unique_block_count;
            }
            return emplace_it;
        }

        void remove_the_least_used_block() {
            std::make_heap(min_heap.begin(), min_heap.end(), min_heap_comparator);
            std::pop_heap(min_heap.begin(), min_heap.end(), min_heap_comparator);
            int64_t k = min_heap.back()->mapped_offset;
            auto removed_count = hash_table.erase(k);
            assert(removed_count == 1);
            min_heap[--heap_end_pos].reset();
            ++m_cache_rebuild_count;
        }
    };
}