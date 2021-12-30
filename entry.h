#pragma once

#include <type_traits>
#include <cstdint>
#include <optional>

#include "utils.h"

template <typename K, typename V>
struct Entry {
    typedef typename std::conditional_t<std::is_arithmetic_v<V>, V, const uint8_t*> ValueType;

    const ValueType value;
    const int size;
    const K key;
private:
    Entry(std::true_type *): value(0), size(0), key(-1)  {}

    Entry(std::false_type *): value(nullptr), size(0), key(-1)  {}

public:
    Entry() : Entry((std::is_arithmetic<V> *)nullptr) {}

    Entry(const K &key, const ValueType value, const int size) : value(value), size(size), key(key) {}

    bool is_dummy() {
        return (key == -1) && (value == 0);
    }

    std::optional<V> get_value() const {
        if (!value)
            return std::nullopt;
        if constexpr (is_string_v<V>) {
            auto* casted_value = reinterpret_cast<const typename V::value_type*>(value);
            return V(casted_value, size);
        } else {
            return V(value);
        }
    }

    bool has_value(const V& other) {
        if constexpr (is_string_v<V>) {
            auto* casted_value = reinterpret_cast<const typename V::value_type*>(value);
            const V &v = V(casted_value, size);
            return (size == static_cast<decltype(size)>(other.size())) && (v == other);
        } else {
            return value == other;
        }
    }
};