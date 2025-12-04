# AST Core Test CMake Configuration

cmake_minimum_required(VERSION 3.10)
project(AST_Core_Test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable all warnings and treat as errors in debug mode
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-Wall -Wextra -g -O0)
else()
    add_compile_options(-Wall -Wextra -O2)
endif()

# Include directories
include_directories(
    ../../include
    ../../src
)

# Test executable for AST core functionality
add_executable(test_ast_core
    ../../src/sql_parser/ast/source_location.cpp
    ../../src/sql_parser/ast/ast_node.cpp
    ../../src/sql_parser/errors/error_core.cpp
    ../../tests/sql_parser/ast_core_test.cpp
)
