cmake_minimum_required(VERSION 3.17)
project(nsmbw-inspector)

set(CMAKE_CXX_STANDARD 20)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

add_subdirectory("libs")
add_subdirectory("src")
add_subdirectory("data")