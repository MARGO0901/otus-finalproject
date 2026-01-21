cmake_minimum_required(VERSION 3.12)

find_package(Boost REQUIRED COMPONENTS unit_test_framework)

add_executable(device_registry_tests test_device_registry.cpp)

target_include_directories(device_registry_tests SYSTEM PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(device_registry_tests PRIVATE Boost::unit_test_framework)

enable_testing()

add_test(NAME DeviceRegistryTests COMMAND device_registry_tests)
