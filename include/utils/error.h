#pragma once

#include <string>

namespace btree::error_msg {
    constexpr std::string_view wrong_order_msg =
            "The order(T) for your tree doesn't equal to the order(T) used in storage: ";

    constexpr std::string_view wrong_key_size_msg =
            "The sizeof(KEY) for your tree doesn't equal to the sizeof(KEY) used in storage: ";

    constexpr std::string_view wrong_value_type_msg =
            "The VALUE_TYPE for your tree doesn't equal to the VALUE_TYPE used in storage: ";

    constexpr std::string_view wrong_element_size_msg =
            "The ELEMENT_SIZE for your tree doesn't equal to the ELEMENT_SIZE used in storage: ";
}