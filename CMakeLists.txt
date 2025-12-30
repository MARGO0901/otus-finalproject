cmake_minimum_required(VERSION 3.12)

set(PATCH_VERSION "1" CACHE INTERNAL "Patch version")
set(PROJECT_VERSION 0.0.${PATCH_VERSION})

project(penguin VERSION ${PROJECT_VERSION})

add_executable(penguin main.cpp game.cpp device.cpp malfunction.cpp penguin.cpp)

set_target_properties(penguin PROPERTIES 
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON 
)

install(TARGETS penguin RUNTIME DESTINATION bin)

set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_CONTACT example@example.com)
include(CPack)