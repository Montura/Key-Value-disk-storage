#pragma once

#include <string>

static_assert(sizeof(int64_t) == sizeof(size_t));

template <typename T>
struct is_string {
    static constexpr const bool value = false;
};

template <typename CharT, typename Traits, typename Alloc>
struct is_string <std::basic_string<CharT, Traits, Alloc>> {
    static constexpr const bool value = true;
};

template <typename T>
inline constexpr bool is_string_v = is_string<T>::value;

template <typename T>
struct is_vector {
    static constexpr const bool value = false;
};

template <typename Tp, typename Alloc>
struct is_vector <std::vector<Tp, Alloc>> {
    static constexpr const bool value = true;
};

template <typename T>
inline constexpr bool is_vector_v = is_vector<T>::value;


template <typename T>
using to_string = std::basic_string<T> (*)(int i);

template <typename V>
constexpr uint8_t get_element_size() {
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
constexpr uint8_t get_value_type_code() {
    if constexpr (std::is_arithmetic_v<V>) {
        return 0;
    }
    if constexpr (is_string_v<V> || is_vector_v<V>) {
        return 1;
    } else {
        return -1;
    }
}


template <typename PtrT>
inline constexpr uint8_t* cast_to_uint8_t_data(PtrT t) {
    return reinterpret_cast<uint8_t *>(t);
}

template <typename PtrT>
inline constexpr const uint8_t* cast_to_const_uint8_t_data(PtrT t) {
    return reinterpret_cast<const uint8_t *>(t);
}