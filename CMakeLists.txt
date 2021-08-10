cmake_minimum_required(VERSION 3.17)
project(nsmbw-inspector)

set(CMAKE_CXX_STANDARD 20)

set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory("libs")
add_subdirectory("src")
add_subdirectory("data")

add_dependencies(nsmbw-inspector copyAssets)