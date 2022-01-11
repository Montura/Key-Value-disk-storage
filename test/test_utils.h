#pragma once

#include <cassert>
#include <optional>

#include "storage.h"
#include "utils/utils.h"

namespace tests {
namespace test_utils {
    using namespace utils;

    std::tuple<int, int, int> generate_rand_keys() {
        int r1 = std::rand() % 7 + 1;
        int r2 = std::rand() % 13 + 1;
        int r3 = std::rand() % 17 + 1;
        return std::make_tuple(r1, r2, r3);
    }

    template <typename V>
    struct Data {
        V value;
        int32_t len;
    };

    template <typename V>
    class ValueGenerator {
        std::map<int, Data<V>> blob_map;

    public:
        ~ValueGenerator() {
            clear();
        }

        void clear() {
            if constexpr(std::is_pointer_v<V>) {
                for (auto& entry: blob_map) {
                    delete entry.second.value;
                }
                blob_map.clear();
            }
        }

        int64_t map_size() const {
            return blob_map.size();
        }

        const std::map<int32_t, Data<V>>& map() const {
            return blob_map;
        }

        void remove(int32_t key) {
            auto it = blob_map.find(key);

            if (it != blob_map.end()) {
                if constexpr(std::is_pointer_v<V>) {
                    delete it->second.value;
                }
                blob_map.erase(it);
            }
        }

        Data<V> next_value(int key) {
            int rand = key + std::rand() % 31;
            if constexpr (is_string_v<V>) {
                if constexpr(std::is_same_v<typename V::value_type, char>) {
                    auto str = std::to_string(rand) + "abacaba";
                    Data<V> data { str, static_cast<int32_t>(str.size() * sizeof(typename V::value_type)) };
                    blob_map[key] = data;
                    return data;
                } else {
                    auto w_str = std::to_wstring(rand) + L"abacaba";
                    Data<V> data { w_str, static_cast<int32_t>(w_str.size() * sizeof(typename V::value_type)) };
                    blob_map[key] = data;
                    return data;
                }
            } else {
                if constexpr(std::is_pointer_v<V>) {
                    int32_t len = rand % 1000 + 1; // don't allocate more than 1kb for test values
                    auto arr = new char[len + 1];
                    for (int k = 0; k < len; ++k) {
                        arr[k] = 2;
                    }
                    arr[len] = 0;
                    Data<V> data { arr, len };
                    blob_map[key] = data;
                    return data;
                } else {
                    auto val = static_cast<V>(rand);
                    Data<V> data { val, sizeof(val) };
                    blob_map[key] = data;
                    return data;
                }
            }
        }

        template <typename K>
        bool check(K key, const typename btree::Storage<K,V>::VolumeWrapper& volume) {
            auto it = blob_map.find(key);
            if (it == blob_map.end()) {
                return volume.get(key) == std::nullopt;
            }

            return check_value(key, volume.get(key));
        }

        bool check_value(int32_t key, const std::optional<V>& actual_value) {
            Data<V> expected_value = blob_map.find(key)->second;
            if (expected_value.len == 0)
                return actual_value == std::nullopt;

            if constexpr(std::is_pointer_v<V>) {
                V expected = expected_value.value;
                V actual = actual_value.value();
                bool res = true;
                for (size_t k = 0, len = expected_value.len; k < len; ++k) {
                    res &= (expected[k] == actual[k]);
                }
                return res;
            } else {
                return expected_value.value == actual_value.value();
            }
        }
    };
}
}