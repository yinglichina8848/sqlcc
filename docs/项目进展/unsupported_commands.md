# SQLCC 1.0.2 - 不支持的DCL和DDL命令

## 1. DCL命令

### 支持的DCL命令
- CREATE USER
- GRANT
- REVOKE

### 不支持的DCL命令
- ALTER USER
- DROP USER
- CREATE ROLE
- DROP ROLE
- ALTER ROLE
- SET ROLE

## 2. DDL命令

### 支持的DDL命令
- CREATE
- DROP
- ALTER
- CREATE INDEX
- DROP INDEX
- USE

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

例如：
```
ERROR: Command not supported: ALTER USER
ERROR: Command not supported: TRUNCATE TABLE
```