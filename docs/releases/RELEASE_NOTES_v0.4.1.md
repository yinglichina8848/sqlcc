# SQLCC v0.4.1 发布说明

## 主要修复内容

1. **修复SQL解析器JOIN子句解析问题**
   - 在`parseSelect`方法中正确添加了JOIN子句处理逻辑
   - 确保JOIN子句测试`SelectStatementJoinClause`能够正常通过
   - 移除了导致段错误的无效代码

2. **增强SQL解析器功能**
   - 改进了`parseColumnDefinition`方法，支持处理带括号的列类型定义（如VARCHAR(255)）
   - 确保`CreateTableStatement`测试能够正常通过

## 文件变更

- `src/sql_parser/parser.cpp`：修复JOIN子句解析逻辑，增强类型解析能力
- `VERSION`：更新版本号从v0.4.0到v0.4.1

## 兼容性

该版本完全向后兼容v0.4.0版本。