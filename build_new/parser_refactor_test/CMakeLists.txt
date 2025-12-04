# Parser Refactoring Test CMake Configuration

cmake_minimum_required(VERSION 3.10)
project(Parser_Refactor_Test)

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

# Test executable for basic parser functionality
add_executable(test_parser_basic
    ../../src/sql_parser/lexer_new.cpp
    ../../src/sql_parser/token_new.cpp
    ../../src/sql_parser/parser_new.cpp
    ../../tests/sql_parser/parser_new_basic_test.cpp
)

# Integration test with existing AST
add_executable(test_parser_integration
    ../../src/sql_parser/lexer_new.cpp
    ../../src/sql_parser/token_new.cpp
    ../../src/sql_parser/parser_new.cpp
    ../../tests/sql_parser/parser_new_integration_test.cpp
)
