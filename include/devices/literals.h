#pragma once

namespace literals {
    constexpr double operator"" _bar(long double pressure) {
        return static_cast<double>(pressure);
    }

    constexpr double operator"" _celsies(long double temp) {
        return static_cast<double>(temp);
    }

    constexpr double operator"" _amper(long double current) {
        return static_cast<double>(current);
    }

    constexpr double operator"" _mms(long double vibro) {
        return static_cast<double>(vibro);
    }

    constexpr int operator"" _rpm(unsigned long long rpm) {
        return static_cast<int>(rpm);
    }

    constexpr int operator"" _percent(unsigned long long percent) {
        return static_cast<int>(percent);
    }

    constexpr int operator"" _times(unsigned long long onoff) {
        return static_cast<int>(onoff);
    }
}