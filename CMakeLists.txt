cmake_minimum_required (VERSION 3.5)

project (LuaBridge)

set (gtest_force_shared_crt ON CACHE BOOL "Use /MD and /MDd" FORCE)
add_definitions (-DBUILD_GMOCK)
add_subdirectory (third_party/gtest)

add_subdirectory (Source)

add_subdirectory (Tests)
