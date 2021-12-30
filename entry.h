#pragma once

#include <type_traits>
#include <cstdint>
#include <optional>

#include "utils.h"

template <typename K, typename V>
struct Entry {
    typedef typename std::conditional_t<std::is_arithmetic_v<V>, V, const uint8_t*> ValueType;

    const K key;

private:
    const ValueType value;
    const int size;

    explicit Entry(std::true_type *): key(-1), value(0), size(0) {}
    explicit Entry(std::false_type *):  key(-1), value(nullptr), size(0)  {}

public:
    explicit Entry() : Entry((std::is_arithmetic<V> *)nullptr) {}

    Entry(const K &key, const ValueType value, const int size) : key(key), value(value), size(size) {}

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