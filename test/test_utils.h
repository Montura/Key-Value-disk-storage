#pragma once

#include <cassert>
#include <optional>

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

    int32_t get_len_by_idx(int32_t const idx) {
        return std::min(idx + 5, 1000);
    }

    template <typename V>
    class ValueGenerator {
        std::map<int, V> blob_map;

    public:
        ~ValueGenerator() {
            clear();
        }

        void clear() {
            if constexpr(std::is_pointer_v<V>) {
                for (auto& data: blob_map) {
                    delete data.second;
                }
                blob_map.clear();
            }
        }

        int64_t map_size() {
            return blob_map.size();
        }

        const std::map<int32_t, V>& map() {
            return blob_map;
        }

        void remove(int32_t key) {
            auto it = blob_map.find(key);

            if (it != blob_map.end()) {
                if constexpr(std::is_pointer_v<V>) {
                    delete it->second;
                }
                blob_map.erase(it);
            }
        }

        V next_value(int key) {
            int rand = key + std::rand() % 31;
            if constexpr (is_string_v<V>) {
                if constexpr(std::is_same_v<typename V::value_type, char>) {
                    auto val = std::to_string(rand) + "abacaba";
                    blob_map[key] = val;
                    return val;
                } else {
                    auto val = std::to_wstring(rand) + L"abacaba";
                    blob_map[key] = val;
                    return val;
                }
            } else {
                if constexpr(std::is_same_v<V, const char*>) {
                    int len = get_len_by_idx(rand);
                    auto blob = new char[len + 1];
                    for (int k = 0; k < len; ++k) {
                        blob[k] = 2;
                    }
                    blob[len] = 0;
                    blob_map[key] = blob;
                    return blob;
                } else {
                    auto val = static_cast<V>(rand);
                    blob_map[key] = val;
                    return val;
                }
            }
        }

        bool check(int32_t key, const std::optional<V>& actual_value) {
            auto expected = blob_map.find(key)->second;
            if constexpr(std::is_pointer_v<V>) {
                auto* actual = actual_value.value();
                size_t len = get_len_by_idx(key);
                bool res = true;
                for (size_t k = 0; k < len; ++k) {
                    res &= (expected[k] == actual[k]);
                }
                return res;
            } else {
                return expected == actual_value.value();
            }
        }
    };


}
}