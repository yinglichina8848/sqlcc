# Statement Node Test CMake Configuration

cmake_minimum_required(VERSION 3.10)
project(Statement_Node_Test)

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

# Test executable for statement node functionality
add_executable(test_statement_nodes
    ../../src/sql_parser/ast/source_location.cpp
    ../../src/sql_parser/ast/ast_node.cpp
    ../../tests/sql_parser/statement_node_test.cpp
)
