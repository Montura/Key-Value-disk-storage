#pragma once

template <typename K, typename V>
struct Entry {
    K key;
    V value;

    Entry() {}
    Entry(const K &key, const V &value) : key(key), value(value) {}

    void setKeyValue(const K &key, const V &value) {
        this->key = key;
        this->value = value;
    }

    void setValue(const V &value) {
        this->value = value;
    }
};