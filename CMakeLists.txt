cmake_minimum_required (VERSION 3.5)

project (LuaBridge)

include (CMakeDependentOption)

option (LUABRIDGE_CXX17 "Use C++17 standard if supported by compiler" OFF)

if (LUABRIDGE_CXX17)
  set (CMAKE_CXX_STANDARD 17)
else ()
  set (CMAKE_CXX_STANDARD 11)
endif ()

set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)

cmake_dependent_option (LUABRIDGE_TESTING "Build tests" ON
  "CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR" OFF)

add_subdirectory (Source)

if (LUABRIDGE_TESTING)
    set (gtest_force_shared_crt ON CACHE BOOL "Use /MD and /MDd" FORCE)
    add_subdirectory (third_party/gtest)

    enable_testing ()
    add_subdirectory (Tests)
endif ()

add_custom_target (Documentation SOURCES
    CHANGES.md
    README.md
    Doxyfile
    index.html
    Manual.html
)
