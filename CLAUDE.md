# CLAUDE.md

## ⚠️ AI Agent 必需阅读指南

**All AI Agents working on SQLCC MUST read these documents first**:

| Priority | Document | Description | Status |
|----------|----------|-------------|--------|
| 🔴 **P0** | `docs/sdd/SPEC_DRIVEN_DEVELOPMENT.md` | **SDD Spec-Driven Development Guide** | ☐ |
| 🔴 **P0** | `docs/ai_tools/AI_COLLABORATION_GUIDE.md` | **Multi-Agent Parallel Collaboration Guide** | ☐ |
| 🟡 P1 | `docs/ai_tools/AI_DEVELOPMENT_GUIDELINES.md` | AI Development Guidelines | ☐ |
| 🟡 P1 | `docs/ai_tools/CPP_DEVELOPMENT_SPECIFICATION.md` | C++ Development Specification | ☐ |

### SDD Compliance Requirements

**All AI Agents MUST follow these rules**:

1. **Task State Machine**: `OPEN → CLAIMED → WIP → DONE → FROZEN`
2. **Message Protocol**: Use standard messages (TASK_CLAIM, PROGRESS_UPDATE, BLOCKER_NOTIFICATION, TASK_COMPLETE)
3. **Communication Frequency**: Progress update every 30 minutes, blockers immediate
4. **Acceptance Criteria**: Compile Pass → 100% Tests → Coverage达标 → Docs Complete → CHANGELOG Updated

### Multi-Agent Workflow

```
┌─────────────────────────────────────────────────────────────────┐
│                    Multi-Agent Workflow                          │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│   Master Agent: Task decomposition → Progress aggregation       │
│   Developer Agent: Code implementation → Unit testing           │
│   Tester Agent: Test execution → Coverage analysis              │
│   Reviewer Agent: Code review → Quality gate                    │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### Quick Reference

```bash
# Read AI Collaboration Guide
cat docs/ai_tools/AI_COLLABORATION_GUIDE.md

# Read SDD Specification
cat docs/sdd/SPEC_DRIVEN_DEVELOPMENT.md

# Check current task status
cat docs/sdd/refactoring/level2_core/tasks.md
```

---

## Project Overview

SQLCC is an enterprise-grade, AI-driven, memory-safe cloud-native database system that implements complete SQL-92 standard support with a custom storage engine. Built with C++20 and Bazel, it features a comprehensive test suite with 56.1% code coverage.

**Current Version**: v1.3.9
**Language**: C++20
**Build System**: Bazel 8.5.0+
**Test Framework**: Google Test (GTest)
**Coverage Toolchain**: LLVM 20 + Clang 20+

## Build Commands

```bash
# Clean and build the entire project
bazel clean
bazel build //...

# Build specific targets
bazel build //src:sqlcc_server          # Main server executable
bazel build //src/bin:isql_main         # Interactive SQL client
bazel build //src/sql_parser:sql_parser # SQL parser library
bazel build //src/storage_engine:storage_engine  # Storage engine library

# Build tests
bazel build //tests/...
```

## Test Commands

```bash
# Run all tests
bazel test //... --test_output=all

# Run specific test layers
bazel test //tests/level1_foundation/...     # Foundation (config, logger, types, utils)
bazel test //tests/level2_core/...           # Core services (parser, executor)
bazel test //tests/level2_storage_engine/...  # Storage engine and B+ tree
bazel test //tests/level3_transaction_manager/... # Transaction management
bazel test //tests/level4_sql_processing/... # SQL execution
bazel test //tests/level5_network/...         # Network layer

# Run specific test file
bazel test //tests/level1_foundation/config:config_test
bazel test //tests/level2_core_services/sql_parser/lexer:lexer_test

# Run with coverage
bazel coverage --collect_code_coverage //tests/level1_foundation/...
```

## High-Level Architecture

### Execution Strategy Pattern

The SQL execution engine uses a **Strategy Pattern** with specialized executors for different SQL statement types:

- **ExecutionStrategy** (base class): Defines the execution interface with `execute()`, `checkPermission()`, and `validate()` methods
- **DDLExecutionStrategy**: Handles CREATE, ALTER, DROP statements
- **DMLExecutionStrategy**: Handles SELECT, INSERT, UPDATE, DELETE statements
- **DCLExecutionStrategy**: Handles GRANT, REVOKE, user management
- **UtilityExecutionStrategy**: Handles SHOW, USE, DESCRIBE statements

All strategies use `ExecutionContext` for state management and return `ExecutionResult` objects.

### Key Architectural Components

1. **SQL Parser** (`src/sql_parser/`): Lexer → Parser → AST generation, supports SQL-92 standard
2. **Execution Engine** (`src/execution/`): Strategy-based execution with specialized executors
3. **Storage Engine** (`src/storage_engine/`): 8KB fixed-size pages, V3 BufferPool architecture, B+ tree indexing
4. **Transaction Manager** (`src/transaction_manager/`): ACID with WAL, two-phase locking, read-committed isolation
5. **Network Layer** (`src/network/`): AES encryption, TLS/SSL support, MySQL protocol
6. **Permission System** (`src/core/permission_validator.h`): Complete GRANT/REVOKE framework

### Directory Structure

```
src/
├── core/                    # Core services (DatabaseManager, ExecutionContext, UserManager)
├── execution/               # Execution strategies (DDL/DML/DCL/Utility executors)
├── execution_ast/           # AST execution implementation
├── sql_parser/              # Lexer, Parser, AST nodes
│   └── ast/                # AST node definitions
├── storage_engine/          # Storage engine core
│   ├── buffer_pool/         # Page cache (BufferPoolSharded)
│   ├── b_plus_tree/        # B+ tree index implementation
│   ├── disk_manager/        # File I/O operations
│   └── index_manager/      # Index management
├── transaction_manager/     # Transaction coordination
├── network/                 # Client-server communication
├── sql_executor/           # SQL execution engine
├── config_manager/         # Configuration management
├── logger/                  # Logging system
├── exception/               # Exception hierarchy
└── types/                  # Type system
```

## Test Organization (7-Layer Hierarchy)

| Layer | Description | Key Components |
|-------|-------------|---------------|
| Level1_Foundation | Basic infrastructure | config, logger, types, utils |
| Level2_Core_Services | Core services | database manager, SQL parser, execution, permission validator |
| Level2_Storage_Engine | Storage engine | B+ tree, buffer pool, disk manager |
| Level3_Transaction_Manager | Transactions | WAL, concurrency control |
| Level4_SQL_Processing | SQL execution | DDL, DML, DCL executors |
| Level5_Network | Network | MySQL protocol, encryption |
| Level6_Enterprise | Enterprise features | Security, monitoring |

### Recent Test Architecture Improvements (v1.3.9)

- **Level 2 Core Services 重构**: 合并了原 level2_core 和 level2_core_services 目录
- **新增测试组件**: 添加了 permission_validator 测试模块
- **模块化测试结构**: 每个测试组件都有独立的子目录和 BUILD.bazel 文件
- **重复测试清理**: 删除了 6 个重复的测试文件，优化了测试结构

## Important Patterns and Conventions

### Smart Pointer Usage
- Use `std::shared_ptr` for shared ownership (e.g., Page objects in BufferPool)
- Use `std::unique_ptr` for exclusive ownership (e.g., Page allocation)
- Avoid raw pointers in production code

### Header Dependencies
- Prefer forward declarations in headers when possible
- Include complete type definitions for smart pointers to work properly
- Avoid circular dependencies between headers

### Bazel Build Files
- Each directory with source files needs a `BUILD.bazel` file
- Use `cc_library` for libraries, `cc_binary` for executables, `cc_test` for tests
- Specify `deps` for library dependencies
- Use `visibility` to control which targets can depend on a library

### Execution Context
- `ExecutionContext` carries execution state: current user, database, transaction ID, rows affected
- Pass by reference (`ExecutionContext& context`) to avoid copies
- Use getter/setter methods for all state access

### Error Handling
- Use custom exception classes from `src/exception/`
- Set error state in `ExecutionContext` via `set_error(bool, const std::string&)`
- Check errors via `context.has_error()`

## Code Coverage

Current overall line coverage: 56.1%

```bash
# Run Level1-2 coverage tests
./scripts/run_level1_level2_coverage_tests.sh

# Run comprehensive coverage tests
./scripts/run_comprehensive_coverage_tests.sh

# Generate HTML coverage report
./scripts/generate_coverage_html_report.sh

# Check coverage quality with threshold
./scripts/check_coverage_quality.sh --threshold 70
```

## Performance Characteristics

CRUD operations (8-thread concurrent):
- INSERT: 318 ops/sec
- SELECT point: 787-807 ops/sec
- SELECT range: 671-735 ops/sec
- UPDATE: 994-1066 ops/sec
- DELETE: 1138-1190 ops/sec

## Troubleshooting

### Build Errors
```bash
# Clean and rebuild
bazel clean
bazel build //...

# Check build environment
./scripts/validate_build_environment.sh

# Fix include paths
python3 fix_include_paths_systematic.py
```

### Test Failures
```bash
# Run with verbose output
bazel test //... --test_output=all

# Run tests sequentially
bazel test //... --test_strategy=exclusive

# Check test logs
cat bazel-testlogs/tests/level1_foundation/config/config_test/test.log
```

## Configuration

Server configuration is in `config/sqlcc.conf`. Key settings:
- `server.port`: Default 8080
- `server.threads`: Default 4
- `storage.data_dir`: Default "./data"
- `storage.buffer_pool_size`: Default 1024
- `storage.page_size`: Default 8192

## SQL-92 Compliance

SQLCC provides 100% SQL-92 standard support for:
- DDL: CREATE, ALTER, DROP (tables, indexes, users)
- DML: SELECT, INSERT, UPDATE, DELETE
- DCL: GRANT, REVOKE
- TCL: BEGIN, COMMIT, ROLLBACK, SAVEPOINT
- Joins: INNER, LEFT, RIGHT, FULL
- Subqueries: Correlated, nested, EXISTS
- Aggregate Functions: SUM, COUNT, AVG, MIN, MAX
- Window Functions: ROW_NUMBER, RANK, SUM, AVG
- Recursive Queries: WITH RECURSIVE
- Set Operations: UNION, INTERSECT, EXCEPT
