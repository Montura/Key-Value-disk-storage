#pragma once

template <typename K, typename V>
struct Entry {
    static const K INVALID_KEY = -1;
//    static const V INVALID_VALUE = -1;

    K key;
    V value;
};