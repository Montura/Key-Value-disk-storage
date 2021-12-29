#pragma once

template <typename K, typename V>
struct Entry {
    K key;
    V value;

    Entry(): key(-1), value(-1) {}

    Entry(const K &key, const V &value) : key(key), value(value) {}

    bool is_dummy() {
        return (key == -1) && (value == -1);
    }
};