I have completed the task of adding comprehensive comments to key components of the codebase, following the high-quality standard set by `src/logger/logger.h` (using WHY/WHAT/HOW layers and textbook references).

**Summary of Changes:**

1.  **`src/network/network_server.h`**:
    *   Added detailed documentation for the `NetworkServer` class, explaining its role as the dual-protocol (MySQL & Native) entry point.
    *   Documented `handle_client` and `handle_mysql_authentication` methods with their implementation mechanisms and design intent.

2.  **`src/sql_parser/parsers/dml/parser_dml.h`**:
    *   Upgraded documentation to explain the Design Intent (WHY), Functional Scope (WHAT), and Implementation Mechanism (HOW) for Data Manipulation Language parsing.
    *   Added textbook references to Chapter 4 (SQL Data Operations).
    *   Detailed specific methods like `parseSelectStatement` and `parseInsertStatement`.

3.  **`src/sql_parser/parsers/ddl/parser_ddl.h`**:
    *   Added comprehensive documentation for Data Definition Language parsing.
    *   Added textbook references to Chapter 3 (SQL Data Definition).
    *   Explained the schema evolution responsibilities of the class.

4.  **`src/sql_parser/parsers/dcl/parser_dcl.h`**:
    *   Documented the RBAC (Role-Based Access Control) parsing logic for GRANT/REVOKE.
    *   Added textbook references to Chapter 10 (Database Security).

5.  **`src/sql_parser/parsers/tcl/parser_tcl.h`**:
    *   Documented Transaction Control Language parsing (BEGIN/COMMIT/ROLLBACK).
    *   Added textbook references to Chapter 8 (OLTP Transaction Processing).
    *   Explained how it enforces ACID properties through syntax parsing.

**Verification:**
*   Executed `bazel build //src:network_headers //src:sql_parser_headers` to ensure all modified headers compile correctly. The build was successful.

The codebase now has consistent and educational documentation across its critical Network and SQL Parser subsystems.