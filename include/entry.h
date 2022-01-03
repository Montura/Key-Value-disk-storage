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

        typedef typename std::conditional_t<std::is_arithmetic_v<V> || std::is_pointer_v<V>,
            identity_type<V>, underlying_type<V>>::type OriginalValueType;

        static constexpr size_t size_of_original_value_type = sizeof(OriginalValueType);

        const K key;
        const ValueType data;
        const int size_in_bytes;

        template <
            typename U = V,
            typename std::enable_if<std::is_arithmetic_v<U>, bool>::type = true>
        explicit Entry()
            : key(-1), data(0), size_in_bytes(0) {}

        template <
            typename U = V,
            typename std::enable_if<is_string_v<U>, bool>::type = true>
        explicit Entry()
            : key(-1), data(nullptr), size_in_bytes(0) {}

        template <
            typename U = V,
            typename std::enable_if<std::is_pointer_v<U>, bool>::type = true>
        explicit Entry()
            : key(-1), data(), size_in_bytes(0) {}


        template <
            typename U = V,
            typename std::enable_if<std::is_arithmetic_v<U>, bool>::type = true>
        Entry(const K &key, const V value) :
            key(key), data(value), size_in_bytes(sizeof(value)) {}


        template <
            typename U = V,
            typename std::enable_if<is_string_v<U>, bool>::type = true>
        Entry(const K &key, const V& value) :
            key(key), data(reinterpret_cast<const ValueType>(value.c_str())),
            size_in_bytes(value.size() * sizeof(typename V::value_type))
        {}

        template <
            typename U = V,
            typename std::enable_if<std::is_pointer_v<U>, bool>::type = true>
        Entry(const K &key, const V& data, const int size) :
            key(key), data(reinterpret_cast<const ValueType>(data)),
            size_in_bytes(size * size_of_original_value_type)
        {
            typedef typename std::remove_pointer_t<V> data_type;
            static_assert(sizeof(data_type) == sizeof(OriginalValueType));
        }

        Entry(const K &key, const ValueType value, const int size) : key(key), data(value), size_in_bytes(size) {}

        bool is_valid() const {
            return (key != -1) && (data != 0);
        }

        std::optional<V> value() const {
            if (!data)
                return std::nullopt;

            return cast_value();
        }

        bool operator==(const Entry& e) const {
            if constexpr (std::is_arithmetic_v<V>) {
                return data == e.data;
            } else {
                bool res = size_in_bytes == e.size_in_bytes;
                for (int i = 0; i < size_in_bytes && res; ++i) {
                    res &= data[i] == e.data[i];
                }
                return res;
            }
        }

        bool operator!=(const Entry& e) const { return !(*this == e); }

        std::optional<V> cast_value() const {
            if constexpr (is_string_v<V>) {
                auto *casted_value = reinterpret_cast<const OriginalValueType *>(data);
                return V(casted_value, size_in_bytes / size_of_original_value_type);
            } else {
                if constexpr(std::is_pointer_v<ValueType>) {
                    return reinterpret_cast<V>(data);
                } else {
                    return V(data);
                }
            }
        }
    };
}