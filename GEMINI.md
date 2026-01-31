# Gemini Code Understanding File for SQLCC

This document provides a comprehensive overview of the SQLCC project, intended to be used as a context for AI-powered development assistance.

## Project Overview

**SQLCC** is a **SQL Cloud Computing Database System** written in C++20. It aims to be a fully-featured, SQL-92 compliant database with a modern, modular architecture.

### Key Features:

*   **Storage Engine:** Manages data storage with an 8KB fixed-page size and a V3 BufferPool architecture.
*   **SQL Parser:** A `ParserNew` architecture that supports the full SQL-92 standard.
*   **Indexing:** B+ Tree implementation for efficient point and range queries.
*   **Transaction Management:** ACID compliance with Write-Ahead Logging (WAL) and two-phase locking.
*   **SQL Support:** Full support for DDL, DML, DCL, and TCL statements, as well as advanced features like JOINs, subqueries, window functions, and recursive queries.
*   **Security:** AES encryption, TLS/SSL support, and a full RBAC permission model.

### Architecture:

The system is composed of the following core components:

*   **Storage Engine:** `storage` and `storage_engine` directories.
*   **SQL Parser:** `sql_parser` directory.
*   **Execution Engine:** `execution`, `execution_ast`, `sql_executor` directories.
*   **Transaction Manager:** `transaction` and `transaction_manager` directories.
*   **Networking:** `network` and `isql_network` directories.

## Building and Running

The project uses **Bazel** as its primary build system, but also contains `CMake` files and scripts that use `cmake` and `make`.

### Bazel Commands:

*   **Build the server:**
    ```bash
    bazel build //src:sqlcc_server
    ```
*   **Run the server:**
    ```bash
    ./bazel-bin/src/sqlcc_server --config=config/sqlcc.conf
    ```
*   **Run all tests:**
    ```bash
    bazel test //...
    ```
*   **Run tests for a specific level:**
    ```bash
    bazel test //tests/level1_foundation/...
    ```
*   **Generate code coverage:**
    ```bash
    bazel coverage //...
    ```

### Using the `run_tests.sh` script:

The `scripts/run_tests.sh` script provides a comprehensive way to build and run tests, and seems to be the preferred method for CI/CD and local testing.

*   **Run all tests:**
    ```bash
    ./scripts/run_tests.sh --all
    ```
*   **Run tests with code coverage:**
    ```bash
    ./scripts/run_tests.sh --coverage
    ```
*   **Run tests in parallel:**
    ```bash
    ./scripts/run_tests.sh --parallel
    ```

## Development Conventions

*   **Language:** C++20
*   **Testing:** The project uses **Google Test (GTest)** for unit and integration testing. Tests are organized in a layered structure from `level1_foundation` to `level7_integration`.
*   **Coding Style:** The code is well-commented, with detailed explanations for the "why", "what", and "how" of the implementation. It makes extensive use of modern C++ features like smart pointers.
*   **Documentation:** The project is extensively documented in the `docs` directory. It also uses Doxygen for API documentation.
*   **Dependencies:** The project uses several external libraries, managed through Bazel's `MODULE.bazel` file. These include:
    *   `googletest`
    *   `bazel_skylib`
    *   `rules_cc`
    *   `rules_proto`
    *   `protobuf`
    *   `rules_foreign_cc`
    *   `platforms`

## AI Agent Development Principles

Based on the documentation in `docs/ai_tools`, any AI agent working on this project must adhere to the following core principles:

### 1. Understand First (FIRST Principle)
Before taking any action, the agent must thoroughly investigate the existing codebase. This involves:
*   Reading relevant source files (`read_file`).
*   Understanding dependencies by inspecting `BUILD.bazel` files.
*   Consulting project documentation (e.g., `AGENTS.md`, `docs/ai_tools`).
*   **Direct modification of code without prior analysis is strictly forbidden.**

### 2. Systematic and Gradual Approach
Changes, especially refactoring, must be systematic, planned, and incremental. The project favors well-defined, staged changes over large, monolithic updates. The impact of any change on the entire system is a primary consideration.

### 3. Respect Existing Conventions
The agent must strictly adhere to the project's established conventions:
*   **Language:** C++20.
*   **Build System:** Only `BUILD.bazel` files should be modified for build configurations. Do not interfere with `CMake` files.
*   **Coding Style:** Follow the established naming conventions (e.g., `PascalCase` for classes/functions, `snake_case` for variables), header file structure, and comment style (especially the `WHY/WHAT/HOW` blocks).
*   **Resource Management:** `std::unique_ptr` and `std::shared_ptr` are mandatory for all resource ownership. **The use of raw pointers for ownership is forbidden.**

### 4. Test-Driven Development (Test Everything)
Testing is a paramount and non-negotiable part of the development workflow.
*   **Layered Testing:** The project has a multi-level testing hierarchy (Level 1 to Level 7) that must be respected.
*   **Coverage is Mandatory:** Strict code coverage targets are enforced, with foundational modules requiring 90-100% coverage.
*   **Add Tests:** Any new feature, bug fix, or modification must be accompanied by corresponding tests.
*   **Validation:** All changes must be validated by running `bazel test` and `bazel coverage`. No code should be considered complete until it passes the relevant tests and quality gates.

### 5. Uphold Quality Gates
There is a strong emphasis on maintaining and improving code quality. The agent must ensure that any change passes all quality gates:
*   The code must compile successfully (`bazel build //...`).
*   All relevant tests must pass (`bazel test //...`).
*   Code coverage must meet or exceed the targets for the modified modules.
*   The change must not introduce performance regressions or memory leaks.