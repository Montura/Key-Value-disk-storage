#pragma once

#include <unordered_map>
#include <unordered_set>

#include "volume.h"

namespace btree::storage {
    template <typename K, typename V, bool SupportMultithreading>
    class StorageBase final {
        class VolumeWrapper;

        typedef std::unordered_set<StorageBase*> StorageMap;
        inline static StorageMap storage_map;

        using VolumeType = std::conditional_t<SupportMultithreading, volume::VolumeMT<K, V>, volume::Volume<K, V>>;
        std::unordered_map<std::string, std::unique_ptr<VolumeType>> volume_map;

    public:
        using VolumeT = VolumeWrapper;

        explicit StorageBase() {
            storage_map.insert(this);
        }

        ~StorageBase() {
            volume_map.clear();
            storage_map.erase(this);
        }

        VolumeT open_volume(const std::string& path, const int16_t user_t) {
            for (const auto& storage: storage_map) {
                auto& curr_volume_map = storage->volume_map;
                auto it = curr_volume_map.find(path);
                if (it != curr_volume_map.end()) {
                    if (this != storage) {
                        throw std::logic_error("Volume " + it->first + " is already opened in another storage!");
                    } else {
                        return VolumeT(it->second.get());
                    }
                }
            }
            auto[pos, success] = volume_map.emplace(path, std::make_unique<VolumeType>(path, user_t));
            return VolumeT(pos->second.get());
        }

        bool close_volume(const VolumeT& v) {
            return volume_map.erase(v.path());
        }

    private:
        class VolumeWrapper {
            VolumeType* const ptr;
            using ValueType = typename VolumeType::ValueType;
        public:
            explicit VolumeWrapper(VolumeType* ptr) : ptr(ptr) {}

            bool exist(const K key) const { return ptr->exist(key); }

            void set(const K key, const ValueType value) { ptr->set(key, value); }

            void set(const K key, const V& value, const int32_t size) { ptr->set(key, value, size); }

            std::optional<V> get(const K key) const { return ptr->get(key); }

            bool remove(const K key) { return ptr->remove(key); }

            std::string path() const { return ptr->path; }
        };
    };
}
namespace btree {
    template <typename K, typename V>
    using Storage = storage::StorageBase<K, V, false>;

    template <typename K, typename V>
    using StorageMT = storage::StorageBase<K, V, true>;
}