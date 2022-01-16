#pragma once

#include <cassert>
#include <optional>
#include <random>
#include <utility>

#include "storage.h"
#include "utils/utils.h"

namespace tests::test_utils {
    using namespace utils;

    template <typename V>
    struct Data {
        V value;
        int32_t len;

        explicit Data() {};

        template <typename U = V, enable_if_t<std::is_arithmetic_v<U>> = true>
        Data(const U& val) : value(val), len(sizeof(val)) {}

        template <typename U = V, enable_if_t<std::is_pointer_v<U>> = true>
        Data(const U& ptr, int32_t size) : value(ptr), len(size) {}

        template <typename U = V, enable_if_t<is_string_v<U>> = true>
        Data(const U& str) : value(str), len(string_size_in_bytes(str)) {}

        template <typename ValueType>
        static constexpr int32_t string_size_in_bytes(const std::basic_string<ValueType>& str) {
            return static_cast<int32_t>(str.size() * sizeof(ValueType));
        }
    };

    template <typename V>
    class ValueGenerator {
        std::map<int32_t, Data<V>> blob_map;
        const int max_blob_size;  // don't allocate more than max_blob_size-bytes for test values
    public:
        std::mt19937 m_rand;
        ValueGenerator(int max_blob_size = 300) :
#if defined(_WIN32) && !defined(_WIN64)
        // todo:
        //  -> problem: exceeding the limit of available VirtualAddress space on x86 on stress test
        //  (at 800mb+ file boost can't allocate mapped_region)
        //  -> solution: don't map the whole file, map only fixed-size file part
        max_blob_size(100)
#else
        max_blob_size(max_blob_size)
#endif
        {}

        ~ValueGenerator() {
            if constexpr(std::is_pointer_v<V>) {
                for (auto& entry: blob_map) {
                    delete entry.second.value;
                }
            }
            blob_map.clear();
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

        Data<V> next_value(int32_t key) {
            int rand = key + m_rand() % 31;
            if constexpr (is_string_v<V>) {
                if constexpr(std::is_same_v<typename V::value_type, char>) {
                    auto str = std::to_string(rand) + "abacaba";
                    auto [it, success] = blob_map.emplace(key, str);
                    return it->second;
                } else {
                    auto w_str = std::to_wstring(rand) + L"abacaba";
                    auto [it, success] = blob_map.emplace(key, w_str);
                    return it->second;
                }
            } else {
                if constexpr(std::is_pointer_v<V>) {
                    auto old_value_it = blob_map.find(key);
                    if (old_value_it != blob_map.end()) {
                        delete old_value_it->second.value;
                        blob_map.erase(old_value_it);
                    }
                    int32_t len = rand % max_blob_size + 1;
                    auto arr = new char[len + 1];
                    for (int k = 0; k < len; ++k) {
                        arr[k] = 2;
                    }
                    arr[len] = 0;
                    auto [it, success] = blob_map.emplace(std::piecewise_construct,
                            std::forward_as_tuple(key), std::forward_as_tuple(arr, len));
                    return it->second;
                } else {
                    auto val = static_cast<V>(rand);
                    auto [it, success] = blob_map.emplace(key, val);
                    return it->second;
                }
            }
        }

        template <typename VolumeT>
        bool check(int32_t key, const VolumeT& volume) {
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