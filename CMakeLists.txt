cmake_minimum_required(VERSION 3.10)
project(FindPathBackend)

set(CMAKE_CXX_STANDARD 17)

# Options pour utiliser libuv ou une autre bibliothèque HTTP
find_package(PkgConfig REQUIRED)

# Utiliser cpp-httplib (solution simple)
add_executable(backend main.cpp)

target_include_directories(backend PRIVATE .)