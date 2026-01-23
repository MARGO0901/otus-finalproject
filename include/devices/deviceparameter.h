#pragma once

#include <string>
#include <functional>
#include <utility>

struct DeviceParameter {
    explicit DeviceParameter(std::string name, std::pair<double, double> opt, std::pair<double, double> range)
        : name_(std::move(name))
        , range_(range)
        , optRange_(std::move(opt)) {}

    bool operator==(const DeviceParameter& other) const {
        return name_ == other.name_;
    }

    std::string name_;
    std::pair<double, double> range_;
    std::pair<double, double> optRange_;
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