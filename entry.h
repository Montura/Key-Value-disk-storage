#pragma once

template <typename K, typename V>
struct Entry {
    static constexpr K INVALID_KEY = -1;
    static constexpr V INVALID_VALUE = -1;

    K key;
    V value;
};