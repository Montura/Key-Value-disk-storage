#pragma once

#include "utils/thread_pool.h"
#include <unordered_map>
#include <algorithm>
#include <atomic>

namespace btree {
    class Block {
        std::atomic<int64_t> m_usage_count = 0;
        bip::mapped_region mapped_region;
        uint8_t* mapped_region_begin;
        bip::offset_t m_pos;
    public:
        const int64_t mapped_offset;

        explicit Block(const std::string& path, int64_t file_offset, const int32_t size = 4096, bip::mode_t mapping_mode = bip::read_write) :
                m_pos(0), mapped_offset(file_offset)
        {
            auto file_mapping = bip::file_mapping(path.data(), mapping_mode);
            mapped_region = bip::mapped_region(file_mapping, mapping_mode, mapped_offset, size);
            mapped_region_begin = cast_to_uint8_t_data(mapped_region.get_address());
        };

        const std::atomic<int64_t>& usage_count() {
            return m_usage_count;
        }

        void add_ref() {
            ++m_usage_count;
        }

        int64_t current_pos() {
            return m_pos;
        }
    };

    template <typename BlockT>
    struct Comparator {
        bool operator()(const std::shared_ptr<BlockT>& lhs, const std::shared_ptr<BlockT>& rhs) {
            return lhs->usage_count() > rhs->usage_count();
        };
    };

    template <typename T>
    class LRUCache {
        using HashTIt = typename std::unordered_map<int64_t, std::shared_ptr<T>>::iterator;

        const std::string path;
        std::vector<std::shared_ptr<T>> min_heap;
        size_t heap_end_pos = 0;
        const Comparator<T> min_heap_comparator;

        std::unordered_map<int64_t, std::shared_ptr<T>> hash_table;
        std::mutex mutex;

        std::atomic<int64_t> m_unique_block_count = 0;
        std::atomic<int64_t> m_cache_rebuild_count = 0;
        std::atomic<int64_t> m_total_lock_ops = 0;
        std::atomic<int64_t> m_total_lock_free_ops = 0;
        const int64_t block_size;
        const int64_t m_cache_size;

    public:

        LRUCache(const int64_t block_size, const int64_t cache_size, const std::string& path) :
            path(path),
            block_size(block_size),
            m_cache_size(cache_size)
        {
            assert(cache_size > 0);
            min_heap.resize(m_cache_size);
        }

        std::shared_ptr<T> on_new_pos(const int64_t pos) {
            int64_t bucket_idx = round_pos(pos);
            const auto find_it = hash_table.find(bucket_idx);
            if (find_it == hash_table.end()) {
                std::unique_lock lock(mutex);
                auto emplace_it = add_new_block(pos, bucket_idx);
                m_total_lock_ops++;
                emplace_it->second->add_ref();
                return emplace_it->second;
            } else {
                m_total_lock_free_ops++;
                find_it->second->add_ref();
                return find_it->second;
            }
        }

        std::vector<std::shared_ptr<Block>> working_set() const {
            return std::vector<std::shared_ptr<Block>>(min_heap.begin(), min_heap.begin() + heap_end_pos);
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
    private:
        HashTIt add_new_block(const int64_t pos, int64_t bucket_idx) {
            const auto&[emplace_it, success] = hash_table.try_emplace(bucket_idx, new T(path, bucket_idx));
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
            auto value_to_remove = min_heap.back();
            int64_t k = round_pos(value_to_remove->mapped_offset);
            auto removed_count = hash_table.erase(k);
            assert(removed_count == 1);
            min_heap[--heap_end_pos].reset();
            ++m_cache_rebuild_count;
        }

        int64_t round_pos(int64_t addr) const {
            return (addr / block_size) * block_size;
        }
    };
}