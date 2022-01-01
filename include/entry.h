#pragma once

#include <type_traits>
#include <cstdint>
#include <optional>

#include "utils/utils.h"

namespace btree {
    template <typename K, typename V>
    class IOManager;

    template <typename K, typename V>
    struct Entry {
        typedef typename std::conditional_t<std::is_arithmetic_v<V>, V, const uint8_t *> ValueType;
        const K key;

    private:
        const ValueType value;
        const int size;

        explicit Entry(std::true_type *) : key(-1), value(0), size(0) {}

        explicit Entry(std::false_type *) : key(-1), value(nullptr), size(0) {}

    public:
        explicit Entry() : Entry((std::is_arithmetic<V> *) nullptr) {}

        Entry(const K &key, const ValueType value, const int size) : key(key), value(value), size(size) {}

        bool is_dummy() {
            return (key == -1) && (value == 0);
        }

        std::optional<V> get_value(IOManager<K,V>& io) const {
            if (!value)
                return std::nullopt;
            return io.read_value(value, size);
        }

        bool has_value(IOManager<K,V>& io, const V &other) const {
            if constexpr (is_string_v<V>) {
                auto v = get_value(io).value();
                return (size == static_cast<decltype(size)>(other.size())) && (v == other);
            } else {
                return value == other;
            }
        }
    };
}