cmake_minimum_required(VERSION 3.21)

project(cpplox)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Default to C++ extensions being off. Clang's modules support have trouble
# with extensions right now and it is not required for any other compiler
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED 20)

set(CMAKE_BUILD_TYPE Debug)

add_executable(cpplox)
target_sources(
    cpplox
    PRIVATE
        main.cpp
        memory.cpp
        debug.cpp
        chunk.cpp
        value.cpp
        vm.cpp
        scanner.cpp
        compiler.cpp
        object.cpp
        table.cpp
)
target_compile_options(cpplox PRIVATE -fsanitize=address)
target_link_options(cpplox PRIVATE -fsanitize=address)
