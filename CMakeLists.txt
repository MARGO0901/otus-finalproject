set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
cmake_minimum_required(VERSION 3.12)

set(PATCH_VERSION "1" CACHE INTERNAL "Patch version")
set(PROJECT_VERSION 0.0.${PATCH_VERSION})

project(penguin VERSION ${PROJECT_VERSION})

find_package(Boost REQUIRED)

add_executable(penguin)

set_target_properties(penguin PROPERTIES 
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON 
)

target_sources(penguin PRIVATE
    src/main.cpp 
    src/game.cpp 
    src/malfunction.cpp 
    src/penguin.cpp 
    src/devices/fan.cpp 
    src/devices/pump.cpp
    src/devices/compressor.cpp
)

target_include_directories(penguin PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(penguin PRIVATE Boost::boost)

install(TARGETS penguin RUNTIME DESTINATION bin)

set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_CONTACT example@example.com)
include(CPack)