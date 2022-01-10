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
        return idx + 5;
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

        V next_value(int i) {
            int val = i + std::rand() % 31;
            if constexpr (is_string_v<V>) {
                if constexpr(std::is_same_v<typename V::value_type, char>) {
                    return std::to_string(val) + "abacaba";
                } else {
                    return std::to_wstring(val) + L"abacaba";
                }
            } else {
                if constexpr(std::is_same_v<V, const char*>) {
                    int len = get_len_by_idx(val);
                    auto blob = new char[len + 1];
                    for (int k = 0; k < len; ++k) {
                        blob[k] = 2;
                    }
                    blob[len] = 0;
                    blob_map[i] = blob;
                    return blob;
                } else {
                    return static_cast<V>(val);
                }
            }
        }
    };

    template <typename V, typename MapIt>
    bool check(int32_t idx, const std::optional<V>& actual_value, MapIt expected_value) {
        bool success = true;
        if constexpr(std::is_pointer_v<V>) {
            auto* expected = expected_value->second;
            auto* actual = actual_value.value();
            size_t len = get_len_by_idx(idx);
            for (size_t k = 0; k < len; ++k) {
                success &= (expected[k] == actual[k]);
            }
        } else {
            success = (expected_value->second == actual_value.value());
        }
        return success;
    }

    template <typename V>
    bool check(int32_t idx, const std::optional<V>& actual_value, const V& expected_value) {
        if constexpr(std::is_pointer_v<V>) {
            auto* expected = expected_value;
            auto* actual = actual_value.value();
            size_t len = get_len_by_idx(idx);
            bool res = true;
            for (size_t k = 0; k < len; ++k) {
                res &= (expected[k] == actual[k]);
            }
            return res;
        } else {
            return expected_value == actual_value.value();
        }
    }
}
}