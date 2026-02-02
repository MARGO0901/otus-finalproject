#pragma once

#include <string>
#include <iomanip>
#include <variant>

#include <boost/format/format_fwd.hpp>
#include <boost/move/utility_core.hpp>

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