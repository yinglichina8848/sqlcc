# DFA Lexer Test CMake Configuration

cmake_minimum_required(VERSION 3.10)
project(DFA_Lexer_Test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(
    ../../include
    ../../src
)

# Source files
set(SOURCES
    ../../src/sql_parser/lexer_new.cpp
    ../../src/sql_parser/token_new.cpp
    ../../tests/sql_parser/lexer_new_test.cpp
)

# Create executable
add_executable(test_dfa_lexer ${SOURCES})

# Link libraries if needed
# target_link_libraries(test_dfa_lexer ${CMAKE_THREAD_LIBS_INIT})
