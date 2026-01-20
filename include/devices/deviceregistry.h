#pragma once

#include "device.h"

#include <memory>
#include <unordered_map>
#include <functional>

class DeviceRegistry {
private:
    //таблица фабрик
    static auto& getFactories() {
        static std::unordered_map<std::string, std::function<std::unique_ptr<Device>()>> factories;
        return factories;
    }
public:
    //регистрация нового типа устройства
    template<typename DeviceType>
    static void registerType(const std::string& name) {
        getFactories()[name] = []() {
            return std::make_unique<DeviceType>();
        };
    }

    //создание устройств по имени
    static std::unique_ptr<Device> create(const std::string& name) {
        auto& factories = getFactories();
        auto it = factories.find(name);
        if (it != factories.end()) {
            return it->second();
        }
        return nullptr;
    }
};