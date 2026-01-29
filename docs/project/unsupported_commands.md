# SQLCC v1.1.3 - 命令支持状态

## 1. DCL命令

### 支持的DCL命令
- CREATE USER
- GRANT
- REVOKE
- DROP USER

### 部分支持的DCL命令
- ALTER USER - 语法解析支持，但功能未完全实现

### 不支持的DCL命令
- CREATE ROLE
- DROP ROLE
- ALTER ROLE
- SET ROLE

## 2. DDL命令

### 支持的DDL命令
- CREATE DATABASE
- DROP DATABASE
- CREATE TABLE
- DROP TABLE
- ALTER TABLE (部分支持)
- CREATE INDEX
- DROP INDEX
- USE DATABASE

### 不支持的DDL命令
- TRUNCATE TABLE
- RENAME TABLE
- CREATE VIEW
- DROP VIEW
- ALTER VIEW
- CREATE SCHEMA
- DROP SCHEMA
- ALTER SCHEMA

## 3. 错误信息格式

对于不支持的命令，系统将返回以下格式的错误信息：
```
ERROR: Command not supported: [命令名称]
```

对于部分支持的命令，系统可能会返回：
```
ERROR: Command partially supported: [命令名称] - [具体不支持的功能]
```

例如：
```
ERROR: Command not supported: ALTER USER
ERROR: Command not supported: TRUNCATE TABLE
ERROR: Command partially supported: ALTER TABLE - 仅支持部分ALTER操作
```

---
**更新时间**: 2025-12-11  
**当前版本**: v1.1.3