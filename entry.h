#pragma once

#include <cstdint>

template <typename K, typename V>
class Entry {
    const uint8_t* value;
    const int size;
public:
    const K key;

    Entry(): value(nullptr), size(0), key(-1)  {}

    Entry(const K &key, const uint8_t* value, const int size) : value(value), size(size),  key(key) {}

    bool is_dummy() {
        return (key == -1) && (value == nullptr);
    }

    V* get_value() const {
        if (!value)
            return nullptr;
        auto casted_value = reinterpret_cast<const typename V::value_type*>(value);
        return new V(casted_value, size);
    }

    bool has_value(const V& other) {
        auto casted_value = reinterpret_cast<const typename V::value_type*>(value);
        const V &v = V(casted_value, size);
        return (size == static_cast<decltype(size)>(other.size())) && (v == other);
    }
};