# SQLCC Bazel标签修复增强报告 v2.0
生成时间: 2025-12-22 03:45:20

## 📊 总体统计
- 处理文件数量: 22
- 修复标签总数: 143
- 错误模式种类: 3

## 🔍 错误模式分析
- **missing_target** (HIGH): 69 次 - 缺少目标名称
- **missing_colon** (UNKNOWN): 69 次 - 未知模式
- **file_extension_in_target** (MEDIUM): 5 次 - 目标名称包含文件扩展名

## 🔧 修复统计
- **file_extension_in_target**: 5 次修复 - 目标名称包含文件扩展名
- **missing_target**: 69 次修复 - 缺少目标名称
- **missing_colon**: 69 次修复 - 未知修复

## 📝 详细修复记录
### 修复 1
- **文件**: /home/liying/sqlcc/BUILD.bazel
- **原始标签**: `//include/utils:logger.cppm`
- **修复后**: `//include/utils:logger`
- **错误类型**: file_extension_in_target
- **原因**: 目标名称包含文件扩展名

### 修复 2
- **文件**: /home/liying/sqlcc/BUILD.bazel
- **原始标签**: `//src/utils:logger_module.cpp`
- **修复后**: `//src/utils:logger_module`
- **错误类型**: file_extension_in_target
- **原因**: 目标名称包含文件扩展名

### 修复 3
- **文件**: /home/liying/sqlcc/include/storage_engine/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 4
- **文件**: /home/liying/sqlcc/include/storage_engine/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 5
- **文件**: /home/liying/sqlcc/include/storage_engine/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 6
- **文件**: /home/liying/sqlcc/include/storage_engine/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 7
- **文件**: /home/liying/sqlcc/include/storage_engine/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 8
- **文件**: /home/liying/sqlcc/include/storage_engine/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 9
- **文件**: /home/liying/sqlcc/include/storage_engine/table_storage/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 10
- **文件**: /home/liying/sqlcc/include/storage_engine/table_storage/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 11
- **文件**: /home/liying/sqlcc/include/storage_engine/table_storage/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 12
- **文件**: /home/liying/sqlcc/include/storage_engine/table_storage/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 13
- **文件**: /home/liying/sqlcc/tests/storage_engine/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 14
- **文件**: /home/liying/sqlcc/tests/storage_engine/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 15
- **文件**: /home/liying/sqlcc/tests/storage_engine/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 16
- **文件**: /home/liying/sqlcc/tests/storage_engine/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 17
- **文件**: /home/liying/sqlcc/tests/storage_engine/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 18
- **文件**: /home/liying/sqlcc/tests/storage_engine/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 19
- **文件**: /home/liying/sqlcc/tests/storage_engine/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 20
- **文件**: /home/liying/sqlcc/tests/storage_engine/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 21
- **文件**: /home/liying/sqlcc/tests/storage_engine/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 22
- **文件**: /home/liying/sqlcc/tests/storage_engine/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 23
- **文件**: /home/liying/sqlcc/tests/storage_engine/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 24
- **文件**: /home/liying/sqlcc/tests/storage_engine/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 25
- **文件**: /home/liying/sqlcc/tests/storage_engine/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 26
- **文件**: /home/liying/sqlcc/tests/storage_engine/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 27
- **文件**: /home/liying/sqlcc/tests/unit/parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 28
- **文件**: /home/liying/sqlcc/tests/unit/parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 29
- **文件**: /home/liying/sqlcc/tests/stubs/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 30
- **文件**: /home/liying/sqlcc/tests/stubs/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 31
- **文件**: /home/liying/sqlcc/tests/components/core/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 32
- **文件**: /home/liying/sqlcc/tests/components/core/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 33
- **文件**: /home/liying/sqlcc/tests/components/core/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 34
- **文件**: /home/liying/sqlcc/tests/components/core/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 35
- **文件**: /home/liying/sqlcc/tests/components/core/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 36
- **文件**: /home/liying/sqlcc/tests/components/core/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 37
- **文件**: /home/liying/sqlcc/tests/components/core/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 38
- **文件**: /home/liying/sqlcc/tests/components/core/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 39
- **文件**: /home/liying/sqlcc/tests/components/core/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 40
- **文件**: /home/liying/sqlcc/tests/components/core/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 41
- **文件**: /home/liying/sqlcc/tests/components/parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 42
- **文件**: /home/liying/sqlcc/tests/components/parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 43
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 44
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 45
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 46
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 47
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 48
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 49
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 50
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 51
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 52
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 53
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 54
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 55
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 56
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 57
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 58
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 59
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 60
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 61
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 62
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 63
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 64
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 65
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 66
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 67
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 68
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 69
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 70
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 71
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 72
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 73
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 74
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 75
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 76
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 77
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 78
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 79
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 80
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 81
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 82
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 83
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 84
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 85
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 86
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 87
- **文件**: /home/liying/sqlcc/tests/sql_executor/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 88
- **文件**: /home/liying/sqlcc/tests/sql_executor/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 89
- **文件**: /home/liying/sqlcc/tests/sql_executor/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 90
- **文件**: /home/liying/sqlcc/tests/sql_executor/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 91
- **文件**: /home/liying/sqlcc/tests/sql_executor/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 92
- **文件**: /home/liying/sqlcc/tests/sql_executor/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 93
- **文件**: /home/liying/sqlcc/tests/sql_executor/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 94
- **文件**: /home/liying/sqlcc/tests/sql_executor/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 95
- **文件**: /home/liying/sqlcc/tests/sql_executor/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 96
- **文件**: /home/liying/sqlcc/tests/sql_executor/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 97
- **文件**: /home/liying/sqlcc/tests/sql_executor/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 98
- **文件**: /home/liying/sqlcc/tests/sql_executor/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 99
- **文件**: /home/liying/sqlcc/tests/sql_executor/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 100
- **文件**: /home/liying/sqlcc/tests/sql_executor/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 101
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **原始标签**: `//src/core:execution_context.cpp`
- **修复后**: `//src/core:execution_context`
- **错误类型**: file_extension_in_target
- **原因**: 目标名称包含文件扩展名

### 修复 102
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **原始标签**: `//src/core:unified_executor.cpp`
- **修复后**: `//src/core:unified_executor`
- **错误类型**: file_extension_in_target
- **原因**: 目标名称包含文件扩展名

### 修复 103
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **原始标签**: `//include/config_manager:config_manager.h`
- **修复后**: `//include/config_manager:config_manager`
- **错误类型**: file_extension_in_target
- **原因**: 目标名称包含文件扩展名

### 修复 104
- **文件**: /home/liying/sqlcc/src/storage_engine/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 105
- **文件**: /home/liying/sqlcc/src/storage_engine/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 106
- **文件**: /home/liying/sqlcc/src/storage_engine/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 107
- **文件**: /home/liying/sqlcc/src/storage_engine/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 108
- **文件**: /home/liying/sqlcc/src/storage_engine/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 109
- **文件**: /home/liying/sqlcc/src/storage_engine/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 110
- **文件**: /home/liying/sqlcc/src/isql_network/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 111
- **文件**: /home/liying/sqlcc/src/isql_network/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 112
- **文件**: /home/liying/sqlcc/src/sqlcc_server/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 113
- **文件**: /home/liying/sqlcc/src/sqlcc_server/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 114
- **文件**: /home/liying/sqlcc/src/logger/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 115
- **文件**: /home/liying/sqlcc/src/logger/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 116
- **文件**: /home/liying/sqlcc/src/transaction/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 117
- **文件**: /home/liying/sqlcc/src/transaction/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 118
- **文件**: /home/liying/sqlcc/src/transaction/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 119
- **文件**: /home/liying/sqlcc/src/transaction/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 120
- **文件**: /home/liying/sqlcc/src/config_manager/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 121
- **文件**: /home/liying/sqlcc/src/config_manager/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 122
- **文件**: /home/liying/sqlcc/src/sql_parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 123
- **文件**: /home/liying/sqlcc/src/sql_parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 124
- **文件**: /home/liying/sqlcc/src/sql_parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 125
- **文件**: /home/liying/sqlcc/src/sql_parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 126
- **文件**: /home/liying/sqlcc/src/sql_parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 127
- **文件**: /home/liying/sqlcc/src/sql_parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 128
- **文件**: /home/liying/sqlcc/src/sql_parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 129
- **文件**: /home/liying/sqlcc/src/sql_parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 130
- **文件**: /home/liying/sqlcc/src/sql_parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 131
- **文件**: /home/liying/sqlcc/src/sql_parser/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 132
- **文件**: /home/liying/sqlcc/src/types/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 133
- **文件**: /home/liying/sqlcc/src/types/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 134
- **文件**: /home/liying/sqlcc/src/transaction_manager/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 135
- **文件**: /home/liying/sqlcc/src/transaction_manager/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 136
- **文件**: /home/liying/sqlcc/src/procedure/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 137
- **文件**: /home/liying/sqlcc/src/procedure/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 138
- **文件**: /home/liying/sqlcc/src/procedure/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 139
- **文件**: /home/liying/sqlcc/src/procedure/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 140
- **文件**: /home/liying/sqlcc/src/procedure/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 141
- **文件**: /home/liying/sqlcc/src/procedure/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

### 修复 142
- **文件**: /home/liying/sqlcc/src/sql_executor/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_target
- **原因**: 缺少目标名称

### 修复 143
- **文件**: /home/liying/sqlcc/src/sql_executor/BUILD.bazel
- **原始标签**: `//include`
- **修复后**: `//include:include`
- **错误类型**: missing_colon
- **原因**: 标签缺少冒号分隔符

## 💡 改进建议
基于本次修复分析，建议采取以下措施:

### 1. 预防措施
- 建立Bazel标签命名规范文档
- 在代码审查中加入标签格式检查
- 开发IDE插件提供实时标签验证

### 2. 自动化工具
- 集成到CI/CD流水线
- 添加git hooks进行预提交检查
- 创建定期扫描任务

### 3. 团队培训
- 组织Bazel最佳实践培训
- 建立常见错误模式文档
- 分享重构经验和教训

---
*此报告由bazel_label_fixer_enhanced_v2.py自动生成*