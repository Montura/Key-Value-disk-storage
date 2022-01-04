#pragma once

#include "utils/utils.h"

namespace btree_test {
namespace utils {
    using namespace btree;

    std::tuple<int, int, int> generate_rand_keys() {
        int r1 = std::rand() % 7 + 1;
        int r2 = std::rand() % 13 + 1;
        int r3 = std::rand() % 17 + 1;
        return std::make_tuple(r1, r2, r3);
    }

    template <typename T>
    using generator = T (*)(int i);

    static constexpr int32_t BLOB_SIZE = 5;

    int32_t get_len_by_idx(int32_t const idx) {
        return idx + 5;
    }

    template <typename V>
    generator<V> create_value_generator() {
        generator<V> r;
        if constexpr (is_string_v < V >) {
            if constexpr(std::is_same_v < typename V::value_type, char >) {
                r = +[](int i) -> V { return std::to_string(i + 65) + "abacaba"; };
            } else {
                r = +[](int i) -> V { return std::to_wstring(i + 65) + L"abacaba"; };
            }
        } else {
            if constexpr(std::is_same_v<V, const char*>) {
                r = +[](int i) -> V {
                    int len = get_len_by_idx(i);
                    auto blob = new char[len];
                    for (int k = 0; k < len; ++k) {
                        blob[k] = 2;
                    }
                    return blob;
                };
            } else {
                r = +[](int i) -> V { return i + 65; };
            }
        }
        return r;
    }

    template <typename V, typename MapIt>
    int64_t check(int32_t idx, const std::optional <V>& actual_value, MapIt expected_value, const int64_t counter) {
        if (actual_value.has_value()) {
            if constexpr(std::is_pointer_v < V >) {
                auto* expected = expected_value->second;
                auto* actual = actual_value.value();
                size_t len = utils::get_len_by_idx(idx);
                for (size_t k = 0; k < len; ++k) {
                    assert(expected[k] == actual[k]);
                }
            } else {
                assert(expected_value->second == actual_value.value());
            }
            return counter + 1;
        } else {
            assert(actual_value == std::nullopt);
        }
        return counter;
    }
}
}