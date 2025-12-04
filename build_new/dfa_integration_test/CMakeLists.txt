# DFA Lexer Integration Test CMake Configuration

cmake_minimum_required(VERSION 3.10)
project(DFA_Integration_Test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable all warnings and treat as errors in debug mode
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-Wall -Wextra -Werror -g -O0)
else()
    add_compile_options(-Wall -Wextra -O2)
endif()

# Include directories
include_directories(
    ../../include
    ../../src
)

# Source files - integrate new DFA lexer with existing parser
set(SOURCES
    # New DFA lexer implementation
    ../../src/sql_parser/lexer_new.cpp
    ../../src/sql_parser/token_new.cpp

    # Existing parser (updated to use new tokens)
    ../../src/sql_parser/parser.cpp

    # Integration test
    ../../tests/sql_parser/lexer_integration_test.cpp
)

# Create executable
add_executable(test_dfa_integration ${SOURCES})

# Link libraries if needed
# target_link_libraries(test_dfa_integration ${CMAKE_THREAD_LIBS_INIT})

# Add test target
add_test(NAME DFAIntegrationTest COMMAND test_dfa_integration)
