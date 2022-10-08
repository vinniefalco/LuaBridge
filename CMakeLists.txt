cmake_minimum_required(VERSION 3.5)

project(LuaBridge)

include(CMakeDependentOption)

option(LUABRIDGE_CXX17 "Use C++17 standard if supported by compiler" OFF)

if(LUABRIDGE_CXX17)
    set(CMAKE_CXX_STANDARD 17)
else()
    set(CMAKE_CXX_STANDARD 11)
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

cmake_dependent_option(LUABRIDGE_TESTING "Build tests" ON
    "CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR" OFF
)

if(WIN32)
    add_compile_options(/MP)
endif()

add_subdirectory(Source)

if(LUABRIDGE_TESTING)
    find_package(GTest)
    if(NOT GTest_FOUND)
        set(gtest_force_shared_crt ON CACHE BOOL "Use /MD and /MDd" FORCE)
        add_subdirectory(third_party/gtest EXCLUDE_FROM_ALL)
        add_library(GTest::gtest ALIAS gtest)
    endif()

    enable_testing()
    add_subdirectory(Tests)
endif()

add_custom_target(Documentation SOURCES
    .github/workflows/cmake.yml
    CHANGES.md
    README.md
    Doxyfile
    index.html
    Manual.html
)
