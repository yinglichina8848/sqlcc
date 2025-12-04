# DFA Lexer Test Suite

This directory contains the test suite for the new DFA-based SQL lexical analyzer implementation.

## Files

- `CMakeLists.txt` - CMake build configuration
- `Makefile` - GNU Make build configuration
- `benchmark_test.cpp` - Performance benchmark comparing old vs new lexer
- `README.md` - This documentation file

## Building and Testing

### Using Make
```bash
# Build the test suite
make

# Run the tests
make test

# Clean build artifacts
make clean
```

### Using CMake
```bash
# Configure
cmake .

# Build
make

# Run tests
./test_dfa_lexer
```

### Manual Compilation
```bash
g++ -std=c++17 -Wall -Wextra -O2 \
    -I../../include -I../../src \
    ../../src/sql_parser/lexer_new.cpp \
    ../../src/sql_parser/token_new.cpp \
    ../../tests/sql_parser/lexer_new_test.cpp \
    -o test_dfa_lexer
```

## Test Coverage

The test suite covers:

1. **Basic Tokenization**
   - Keywords (SELECT, FROM, WHERE, etc.)
   - Operators (=, !=, <, >, etc.)
   - Punctuation (;, (, ), ,, .)

2. **Literals**
   - Integer literals (123, -456)
   - Float literals (3.14, 2.5e10)
   - String literals ('hello', "world")

3. **Identifiers**
   - Standard identifiers (table_name, column123)
   - Unicode identifiers (扩展ASCII字符)

4. **Comments**
   - Single-line comments (`-- comment`)
   - Multi-line comments (`/* comment */`)

5. **Complex SQL**
   - CREATE TABLE statements
   - Complex SELECT queries with JOINs
   - DDL/DML/DCL statements

6. **Position Tracking**
   - Line and column number accuracy
   - Error location reporting

## Performance Benchmark

The benchmark test compares the DFA lexer against the original lexer implementation:

```bash
g++ -std=c++17 benchmark_test.cpp \
    ../../src/sql_parser/lexer_new.cpp \
    ../../src/sql_parser/lexer.cpp \
    ../../src/sql_parser/token_new.cpp \
    ../../src/sql_parser/token.cpp \
    -I../../include -o benchmark
./benchmark
```

## Architecture

The DFA lexer uses:

- **State Machine**: 12 distinct states for different token types
- **Transition Table**: Efficient character-to-state mapping
- **Unicode Support**: Extended ASCII character recognition
- **Comprehensive Keywords**: 80+ SQL keywords across DDL/DML/DCL/TCL
- **Error Recovery**: Graceful handling of invalid input

## Integration Notes

To integrate the new DFA lexer into the main SQLCC system:

1. Replace `Lexer` with `LexerNew` in parser headers
2. Update include paths
3. Rebuild and test the entire system
4. Run full regression tests

## Performance Expectations

- **Memory**: Lower memory footprint due to table-driven approach
- **Speed**: Faster tokenization for complex SQL statements
- **Maintainability**: Easier to extend with new token types
- **Reliability**: Deterministic behavior with comprehensive error handling
