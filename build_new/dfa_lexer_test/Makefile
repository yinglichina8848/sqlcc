# DFA Lexer Test Makefile

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -I../../include -I../../src

# Source files
SOURCES = ../../src/sql_parser/lexer_new.cpp \
          ../../src/sql_parser/token_new.cpp \
          ../../tests/sql_parser/lexer_new_test.cpp

# Object files
OBJECTS = $(SOURCES:.cpp=.o)

# Target executable
TARGET = test_dfa_lexer

# Default target
all: $(TARGET)

# Link executable
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(OBJECTS) $(TARGET)

# Run tests
test: $(TARGET)
	./$(TARGET)

# Phony targets
.PHONY: all clean test
