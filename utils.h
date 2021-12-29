#pragma once

#include <string>

template <typename T>
struct is_string {
    static constexpr bool value = false;
};

template <typename CharT, typename Traits, typename Alloc>
struct is_string <std::basic_string<CharT, Traits, Alloc>> {
    static constexpr bool value = true;
};

template <typename T>
inline constexpr bool is_string_v = is_string<T>::value;

template <typename T>
struct is_vector {
    static constexpr bool value = false;
};

template <typename T, typename Alloc>
struct is_vector <std::vector<T, Alloc>> {
    static constexpr bool value = true;
};

template <typename T>
inline constexpr bool is_vector_v = is_vector<T>::value;


template <typename T>
using to_string = std::basic_string<T> (*)(int i);

template <typename V>
constexpr uint8_t value_type_size() {
    if constexpr (std::is_arithmetic_v<V>) {
        return sizeof(V);
    }
    if constexpr (is_string_v<V> || is_vector_v<V>) {
        return sizeof(typename V::value_type);
    } else {
        return 0;
    }
}

template <typename V>
constexpr uint8_t value_type() {
    if constexpr (std::is_arithmetic_v<V>) {
        return 0;
    }
    if constexpr (is_string_v<V> || is_vector_v<V>) {
        return 1;
    } else {
        return -1;
    }
}