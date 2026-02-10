cmake_minimum_required(VERSION 3.12)

find_package(Boost REQUIRED COMPONENTS unit_test_framework)

# Тест 1: Device Registry
add_executable(device_registry_tests test_device_registry.cpp)

target_include_directories(device_registry_tests SYSTEM PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_sources(device_registry_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/devices/device.cpp 
)

target_link_libraries(device_registry_tests PRIVATE Boost::unit_test_framework)

# Тест 2: Observers
add_executable(observer_tests add_remove_observer.cpp)

target_include_directories(observer_tests SYSTEM PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_sources(observer_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/observers/observer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/observers/penguin.cpp
)

target_link_libraries(observer_tests PRIVATE Boost::unit_test_framework)

enable_testing()

add_test(NAME DeviceRegistryTests COMMAND device_registry_tests)
add_test(NAME ObserverTests COMMAND observer_tests)
