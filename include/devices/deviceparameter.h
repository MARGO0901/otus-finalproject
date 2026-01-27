#pragma once

#include <string>
#include <functional>
#include <utility>

struct DeviceParameter {
    explicit DeviceParameter(std::string name, std::pair<double, double> range, std::pair<double, double> opt)
        : name_(std::move(name))
        , range_(std::move(range))
        , optRange_(std::move(opt))
        , currentRange_(optRange_) {}

    bool operator==(const DeviceParameter& other) const {
        return name_ == other.name_;
    }

    std::string name_;
    std::pair<double, double> range_;           // Абсолютный диапазон
    std::pair<double, double> optRange_;        // Нормальный рабочий диапазон
    std::pair<double, double> currentRange_;    // Текущие границы (могут меняться при неисправности)
};

// Специализация std::hash для DeviceParameter
namespace std {
    template<>
    struct hash<DeviceParameter> {
        std::size_t operator()(const DeviceParameter &param) const {
            return std::hash<std::string>()(param.name_);
        }
    };
}