#pragma once

#include <string>
#include <mutex>

#include "io/io_manager.h"
#include "btree_impl/btree.h"

namespace btree::volume {
    template <typename K, typename V>
    class Volume final {
        IOManager <K, V> io;
        BTree <K, V> btree;
    public:
        using ValueType = typename BTree<K,V>::ValueType;
        const std::string path;

        explicit Volume(const std::string& path, const int16_t order) : io(path, order), btree(order, io), path(path) {}

        bool exist(const K key) {
            return btree.exist(io, key);
        }

        void set(const K key, const ValueType value) {
            btree.set(io, key, value);
        }

        void set(const K key, const V& value, const int32_t size) {
            btree.set(io, key, value, size);
        }

        std::optional <V> get(const K key) {
            return btree.get(io, key);
        }

        bool remove(const K key) {
            return btree.remove(io, key);
        }
    };

    /** Volume with coarse-grained locks for multithreading usage */
    template <typename K, typename V>
    class VolumeMT final {
        Volume<K, V> volume;
        std::mutex mutex_;
    public:
        using ValueType = typename Volume<K,V>::ValueType;
        const std::string path;

        VolumeMT(const std::string& path, int16_t order) : volume(path, order) {}

        bool exist(const K key) {
            std::scoped_lock lock(mutex_);
            return volume.exist(key);
        }

        void set(const K key, const ValueType value) {
            std::scoped_lock lock(mutex_);
            volume.set(key, value);
        }

        void set(const K key, const V& value, const int32_t size) {
            std::scoped_lock lock(mutex_);
            volume.set(key, value, size);
        }

        std::optional <V> get(const K key) {
            std::scoped_lock lock(mutex_);
            return volume.get(key);
        }

        bool remove(const K key) {
            std::scoped_lock lock(mutex_);
            return volume.remove(key);
        }
    };
}