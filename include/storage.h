#pragma once

#include <unordered_map>

#include "entry.h"
#include "io/io_manager.h"
#include "btree_impl/btree.h"

namespace btree {
    template <typename K, typename V>
    class Storage final {
        struct Volume;
        std::map<std::string, std::unique_ptr<Volume>> volume_map;
    public:
        explicit Storage() {
        }

        Volume& open_volume(const std::string& path, const int16_t user_t) {
            auto[it, success] = volume_map.template emplace(path, std::make_unique<Volume>(path, user_t));
            return *it->second;
        }

        bool close_volume(const Volume& v) {
            return volume_map.remove(v.path);
        }

    private:
        struct Volume final {
            const std::string path;
            IOManager<K, V> io;
            BTree<K, V> tree;

            explicit Volume(const std::string& path, const int16_t user_t) :
                    path(path),
                    io(path, user_t),
                    tree(user_t, io) {
            }

            bool exist(const K& key) {
                return tree.exist(io, key);
            }

            void set(const K& key, const V& value) {
                tree.set(io, key, value);
            }

            void set(const K& key, const V& value, const int32_t size) {
                tree.set(io, key, value, size);
            }

            std::optional<V> get(const K& key) {
                return tree.get(io, key);
            }

            bool remove(const K& key) {
                return tree.remove(io, key);
            }
        };
    };
}