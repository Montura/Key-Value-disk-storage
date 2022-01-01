#pragma once

#include <string>

namespace btree {
#if _WIN64 || __amd64__
    static_assert(sizeof(int64_t) == sizeof(size_t));
#else
    static_assert(sizeof(int32_t) == sizeof(size_t));
#endif
    template<typename T>
    void shift_right_by_one(std::vector <T> &v, const int32_t from, const int32_t to) {
        for (auto i = from; i > to; --i) {
            v[i] = v[i - 1];
        }
    }

    template<typename T>
    void shift_left_by_one(std::vector <T> &v, const int32_t from, const int32_t to) {
        for (auto i = from; i < to; ++i) {
            v[i - 1] = v[i];
        }
    }

    template<typename T>
    struct is_string {
        static constexpr bool value = false;
    };

    template<typename CharT, typename Traits, typename Alloc>
    struct is_string<std::basic_string<CharT, Traits, Alloc>> {
        static constexpr bool value = true;
    };

    template<typename T>
    inline constexpr bool is_string_v = is_string<T>::value;

    template<typename T>
    struct is_vector {
        static constexpr bool value = false;
    };

    template<typename T, typename Alloc>
    struct is_vector<std::vector<T, Alloc>> {
        static constexpr bool value = true;
    };

    template<typename T>
    inline constexpr bool is_vector_v = is_vector<T>::value;


    template<typename T>
    using to_string = std::basic_string<T> (*)(int i);

    template<typename V>
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

    template<typename V>
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


    template<typename PtrT>
    constexpr uint8_t *cast_to_uint8_t_data(PtrT t) {
        return reinterpret_cast<uint8_t *>(t);
    }

    template<typename PtrT>
    constexpr const uint8_t *cast_to_const_uint8_t_data(PtrT t) {
        return reinterpret_cast<const uint8_t *>(t);
    }
}