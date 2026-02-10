#pragma once

#include <string>
#include <iomanip>
#include <variant>

#include <boost/format/format_fwd.hpp>
#include <boost/move/utility_core.hpp>

// Функция для корректного вывода значений
inline std::string variantToString(const std::variant<double, int>& value) {
    return std::visit([](auto&& val) -> std::string {
                std::ostringstream oss;
                if constexpr(std::is_same_v<std::decay_t<decltype(val)>, double>) {
                    oss << std::fixed << std::setprecision(1) << val;
                }
                else if constexpr (std::is_same_v<std::decay_t<decltype(val)>, int> ) {
                    oss << val;
                }
                return oss.str();
            },
            value);
}

// Функция для подсчета символов в UTF-8 строке
inline size_t utf8_strlen(const std::string& str) {
    size_t len = 0;
    for (unsigned char c : str) {
        if ((c & 0xC0) != 0x80) len++;
    }
    return len;
}


// Функция для добавления отступов до нужной ширины
inline std::string pad_to_width(const std::string& str, size_t target_width) {
    size_t current_width = utf8_strlen(str);
    if (current_width >= target_width) return str;
    
    return str + std::string(target_width - current_width, ' ');
}