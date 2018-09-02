cmake_minimum_required(VERSION 3.5)

project(LuaBridge)

add_definitions (-DBUILD_GMOCK)
add_subdirectory (third_party/gtest)

add_subdirectory (Source)

add_subdirectory (Tests)
