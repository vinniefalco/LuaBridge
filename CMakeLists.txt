cmake_minimum_required (VERSION 3.5)

project (LuaBridge)

set (CMAKE_CXX_STANDARD 11)

option (LUABRIDGE_NO_CXX11 "Use C++11 standard if supported by compiler" OFF)

if (LUABRIDGE_NO_CXX11)
  add_definitions (-DLUABRIDGE_NO_CXX11)
endif ()

set (gtest_force_shared_crt ON CACHE BOOL "Use /MD and /MDd" FORCE)
add_subdirectory (third_party/gtest)

add_subdirectory (Source)

add_subdirectory (Tests)
