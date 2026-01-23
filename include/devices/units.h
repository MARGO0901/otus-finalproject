#pragma once

namespace units {
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

    constexpr double operator"" _rpm(long double rpm) {
        return static_cast<double>(rpm);
    }

    constexpr double operator"" _percent(long double percent) {
        return static_cast<double>(percent);
    }

    constexpr double operator"" _times(long double onoff) {
        return static_cast<double>(onoff);
    }
}