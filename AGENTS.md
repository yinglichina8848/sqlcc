# AGENTS.md - SQLCC Coding Guidelines for AI Agents

## Project Overview
SQLCC is a C++20 database system using Bazel build system, Google Test, and LLVM coverage tools.

## Build Commands

### Bazel Build
```bash
# Build all targets
bazel build //...

# Build specific target
bazel build //src/core:core
bazel build //src/storage_engine:storage_engine

# Clean build
bazel clean
```

### Testing
```bash
# Run all tests
bazel test //...

# Run specific test target
bazel test //tests/level2_storage_engine/b_plus_tree:bplus_tree_fast

# Run single test file (via test target)
bazel test //tests/level2_storage_engine/b_plus_tree:simple_bplus_tree_test

# Run tests with output
bazel test //tests/... --test_output=all

# Run tests with tag filter
bazel test //tests/... --test_tag_filters=foundation
```

### Coverage
```bash
# Generate coverage report
bazel coverage //...

# Coverage with specific target
bazel coverage //tests/level2_storage_engine/b_plus_tree:all

# Generate HTML report
mkdir -p coverage_report
genhtml --ignore-errors unsupported,inconsistent,corrupt \
  ~/.cache/bazel/_bazel_*/execroot/_main/bazel-out/_coverage/_coverage_report.dat \
  -o coverage_report
```

### Python Tools
```bash
# Run Python tool
cd /home/liying/sqlcc && python3 tools/bazel_code_checker.py

# Fix BUILD file dependencies
python3 tools/bazel_dep_fixer_enhanced.py . --dry-run  # Preview
python3 tools/bazel_dep_fixer_enhanced.py .            # Apply fixes
```

## Code Style Guidelines

### C++ Standards
- **Language**: C++20
- **Compiler**: Clang 20+
- **Standard Library**: libc++
- **Build System**: Bazel 8.5.0+

### Naming Conventions
- **Files**: snake_case.cpp, snake_case.h
- **Classes**: PascalCase (e.g., `DatabaseManager`, `BufferPool`)
- **Functions**: PascalCase for public methods (e.g., `Initialize()`, `GetPage()`)
- **Variables**: snake_case (e.g., `buffer_pool_size`, `page_id`)
- **Member variables**: trailing underscore (e.g., `db_path_`, `is_closed_`)
- **Constants**: UPPER_SNAKE_CASE or kPascalCase
- **Namespaces**: lowercase (e.g., `namespace sqlcc`)

### Include Order
1. Corresponding header file (for .cpp files)
2. Project headers (quoted: `#include "path/to/header.h"`)
3. Third-party headers (angled: `#include <gtest/gtest.h>`)
4. System headers (angled: `#include <memory>`, `#include <vector>`)

### Header Guards
Use `#pragma once` (preferred) or traditional include guards:
```cpp
#pragma once
// or
#ifndef SQLCC_MODULE_FILENAME_H
#define SQLCC_MODULE_FILENAME_H
// ...
#endif  // SQLCC_MODULE_FILENAME_H
```

### Smart Pointers
- Use `std::unique_ptr` for exclusive ownership
- Use `std::shared_ptr` for shared ownership
- Use `std::weak_ptr` to break circular references
- Avoid raw pointers for ownership; use for non-owning references only

### Error Handling
- Use exceptions for exceptional cases
- Define custom exception classes in `src/exception/`
- Use RAII for resource management
- Check return values and handle errors explicitly

### Memory Safety
- Prefer smart pointers over raw pointers
- Use RAII wrappers (see `include/storage_engine/table_storage/page_raii.h`)
- Avoid manual `new`/`delete`
- Check for null pointers before dereferencing

### Comments
- Use `//` for single-line comments
- Use `/* */` for multi-line comments
- Document public APIs with Doxygen-style comments
- Keep comments in Chinese (project convention)

### Testing
- Use Google Test framework
- Test files: `*_test.cpp` or `test_*.cpp`
- Place tests in `tests/` directory following the level structure
- Tag slow tests with `tags = ["slow"]`
- Tag manual/debug tests with `tags = ["manual"]`

### Bazel BUILD Files
- One BUILD.bazel per directory
- Use `glob()` for source files when appropriate
- Declare visibility explicitly
- Group related tests in `test_suite`
- Add meaningful tags for test categorization

## Project Structure
```
sqlcc/
├── src/           # Source code organized by module
│   ├── core/      # Core database components
│   ├── storage_engine/  # Storage engine implementation
│   ├── sql_parser/      # SQL parsing
│   └── ...
├── include/       # Public headers
├── tests/         # Test files organized by level
│   ├── level1_foundation/
│   ├── level2_storage_engine/
│   └── ...
├── tools/         # Development tools (Python)
├── docs/          # Documentation
└── scripts/       # Build and utility scripts
```

## Common Tasks

### Adding a New Test
1. Create test file in appropriate `tests/level*/` directory
2. Add test target to local `BUILD.bazel`
3. Run: `bazel test //tests/path/to:test_target`

### Adding a New Module
1. Create directory under `src/`
2. Add `BUILD.bazel` with `cc_library` target
3. Update dependencies in dependent modules
4. Run: `python3 tools/bazel_dep_fixer_enhanced.py .`

### Debugging Build Issues
1. Check dependencies: `python3 tools/bazel_code_checker.py`
2. Fix includes: `python3 tools/bazel_include_fixer.py`
3. Verbose build: `bazel build //... --verbose_failures`

## CI/CD Integration
GitHub Actions workflows in `.github/workflows/`:
- `ci.yml`: Build and test on push/PR
- `coverage.yml`: Generate coverage reports
- Run locally before pushing: `bazel test //... --test_output=errors`
