cmake_minimum_required(VERSION 3.10)

project(MyCache)

set(CMAKE_CXX_STANDARD 17)

add_executable(
    Cache
    src/main.cpp
)

target_include_directories(
    Cache
    PRIVATE
    include
)


