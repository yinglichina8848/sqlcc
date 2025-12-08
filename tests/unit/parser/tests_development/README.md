# 开发阶段解析器测试文件

## 概述
这个目录包含SQLCC项目开发阶段的解析器测试文件。这些文件主要用于开发和调试SQL解析器、词法分析器和令牌系统。

## 文件说明

### 冒号解析测试
- `test_colon_simple.cpp` - 测试冒号":"的词法分析功能
- `test_colon_parsing.cpp` - 测试冒号在上下文中的解析行为

### 百分号解析测试  
- `test_percent_simple.cpp` - 测试百分号"%"的基本解析功能
- `test_percent_operator.cpp` - 测试百分号在SQL语句中的使用

### 调试工具
- `debug_token_types.cpp` - 显示各种令牌类型的数值
- `debug_lexer_simple.cpp` - 简单的词法分析器调试工具
- `debug_lexer_output.cpp` - 词法分析器输出调试工具

## 使用说明

这些文件可以从这个目录编译和运行。例如：

```bash
# 编译冒号测试
g++ -std=c++17 -I ../../../../include -I ../../../../build/include \
    test_colon_simple.cpp ../../../../src/sql_parser/lexer_new.cpp \
    ../../../../src/sql_parser/token_new.cpp -o test_colon_simple

# 运行测试
./test_colon_simple
```

## 注意事项

1. 这些是开发阶段的测试文件，主要用于验证特定功能的正确性
2. 在生产环境中，建议使用正式的单元测试框架
3. 所有文件都遵循SQLCC的编码规范和命名约定
4. 测试文件可以独立编译运行，不需要整个项目构建

## 与正式测试的关系

这些测试文件对应于正式单元测试中的以下测试：
- `test_colon_simple.cpp` -> `parser/` 目录下的冒号相关测试
- `test_percent_simple.cpp` -> `parser/` 目录下的百分号相关测试
- `test_ddl_functionality.cpp` -> `database_manager_test.cpp` 扩展测试