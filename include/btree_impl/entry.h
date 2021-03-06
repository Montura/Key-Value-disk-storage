#pragma once

#include <optional>

#include "utils/utils.h"

namespace btree::entry {
    template <typename K, typename V>
    class Entry {
        static constexpr bool V_is_arithmetic = std::is_arithmetic_v<V>;
        static constexpr bool V_is_pointer = std::is_pointer_v<V>;
        static constexpr bool V_is_identity = V_is_arithmetic || V_is_pointer;

        using OriginalValueType = typename conditional_t<V_is_identity, identity_type<V>, underlying_type<V>>::type;
        static constexpr size_t size_of_original_value_type = sizeof(OriginalValueType);
    public:
        using ValueType = conditional_t<V_is_arithmetic, V, const uint8_t*>;

        const K key;
        const ValueType data;
        const int32_t size_in_bytes;

        template <typename U = V, enable_if_t<std::is_arithmetic_v<U>> = true>
        explicit Entry() :
                key(-1),
                data(0),
                size_in_bytes(0) {}

        template <typename U = V, enable_if_t<is_string_v<U>> = true>
        explicit Entry() :
                key(-1),
                data(nullptr),
                size_in_bytes(0) {}

        template <typename U = V, enable_if_t<std::is_pointer_v<U>> = true>
        explicit Entry() :
                key(-1),
                data(),
                size_in_bytes(0) {}

        template <typename U = V, enable_if_t<std::is_arithmetic_v<U>> = true>
        Entry(const K key, const V value) :
                key(key),
                data(value),
                size_in_bytes(sizeof(value)) {}

        template <typename U = V, enable_if_t<is_string_v<U>> = true>
        Entry(const K key, const V& value) :
                key(key),
                data(reinterpret_cast<const ValueType>(value.c_str())),
                size_in_bytes(static_cast<int32_t>(value.size() * sizeof(typename V::value_type))) {}

        template <typename U = V, enable_if_t<std::is_pointer_v<U>> = true>
        Entry(const K key, const V& value, const int32_t size) :
                key(key),
                data(reinterpret_cast<const ValueType>(value)),
                size_in_bytes(size)
        {
            using data_type = typename std::remove_pointer_t<V>;
            static_assert(sizeof(data_type) == sizeof(OriginalValueType));
        }

        Entry(const K key, const ValueType value, const int size) :
                key(key),
                data(value),
                size_in_bytes(size) {}

        bool is_valid() const {
            return (key != -1) && (size_in_bytes != 0);
        }

        std::optional<V> value() const {
            if (!size_in_bytes)
                return std::nullopt;

            return cast_value();
        }

        template <typename U = V, enable_if_t<std::is_arithmetic_v<U>> = true>
        bool operator==(const Entry& e) const {
            return data == e.data;
        }

        template <typename U = V, enable_if_t<is_string_v<U> || std::is_pointer_v<U>> = true>
        bool operator==(const Entry& e) const {
            bool res = size_in_bytes == e.size_in_bytes;
            for (int i = 0; i < size_in_bytes && res; ++i) {
                res &= data[i] == e.data[i];
            }
            return res;
        }

        bool operator!=(const Entry& e) const { return !(*this == e); }

    private:
        template <typename U = V, enable_if_t<std::is_pointer_v<U>> = true>
        std::optional<V> cast_value() const {
            return reinterpret_cast<V>(data);
        }

        template <typename U = V, enable_if_t<is_string_v<U>> = true>
        std::optional<V> cast_value() const {
            auto* casted_value = reinterpret_cast<const OriginalValueType*>(data);
            return V(casted_value, size_in_bytes / size_of_original_value_type);
        }

        template <typename U = V, enable_if_t<std::is_arithmetic_v<U>> = true>
        std::optional<V> cast_value() const {
            return V(data);
        }
    };
}