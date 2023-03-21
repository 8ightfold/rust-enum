cmake_minimum_required(VERSION 3.23)

add_library(rust_enum_includes INTERFACE)
target_include_directories(rust_enum_includes INTERFACE include)
add_library(rust_enum::rust_enum ALIAS rust_enum_includes)
