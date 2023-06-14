cmake_minimum_required(VERSION 3.21)

set(CMAKE_C_COMPILER /usr/local/opt/llvm/bin/clang)
set(CMAKE_CXX_COMPILER /usr/local/opt/llvm/bin/clang++)

project(cpplox)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED 23)

set(CMAKE_C_STANDARD 99)

set(CMAKE_BUILD_TYPE Debug)

add_executable(cpplox)
target_sources(
    cpplox
    PRIVATE
        main.cpp
        # memory.cpp
        debug.cpp
        chunk.cpp
        value.cpp
        vm.cpp
        scanner.cpp
        compiler.cpp
        object.cpp
        table.cpp
        compiler_common.cpp
        parse_rule.cpp
        parser.cpp
)
target_compile_options(cpplox PRIVATE -fsanitize=address)
target_link_options(cpplox PRIVATE -fsanitize=address)
