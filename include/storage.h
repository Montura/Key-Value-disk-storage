#pragma once

#include <unordered_map>

#include "volume.h"

namespace btree {
    template <typename K, typename V, bool SupportMultithreading>
    struct StorageBase final {
        class VolumeWrapper;

    private:
        using VolumeT = std::conditional_t<SupportMultithreading, VolumeMT<K,V>, Volume<K,V>>;
        std::map<std::string, std::unique_ptr<VolumeT>> volume_map;

    public:
        explicit StorageBase() {}

        VolumeWrapper open_volume(const std::string& path, const int16_t user_t) {
            auto it = volume_map.find(path);
            if (it != volume_map.end()) {
                return VolumeWrapper(it->second.get());
            } else {
                auto [pos, success] = volume_map.emplace(path, std::make_unique<VolumeT>(path, user_t));
                return VolumeWrapper(pos->second.get());
            }
        }

        bool close_volume(const VolumeWrapper& v) {
            return volume_map.erase(v.path());
        }

        class VolumeWrapper {
            VolumeT* const ptr;

        public:
            explicit VolumeWrapper(VolumeT* ptr) : ptr(ptr) {}

            bool exist(const K& key) const { return ptr->exist(key); }

            void set(const K& key, const V& value) { ptr->set(key, value); }

            void set(const K& key, const V& value, const int32_t size) { ptr->set(key, value, size); }

            std::optional <V> get(const K& key) const { return ptr->get(key); }

            bool remove(const K& key) { return ptr->remove(key); }

            std::string path() const { return ptr->path; }
        };
    };

    template <typename K, typename V>
    using Storage = StorageBase<K, V, false>;

    template <typename K, typename V>
    using StorageMT = StorageBase<K, V, true>;
}