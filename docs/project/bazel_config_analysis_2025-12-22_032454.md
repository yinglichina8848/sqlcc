# SQLCC Bazel构建配置分析报告
项目根目录: /home/liying/sqlcc
分析时间: 2025-12-22 03:24:54

## 统计摘要
- 分析的BUILD文件数量: 64
- 发现的问题总数: 437

### 按严重程度统计
- HIGH: 220
- MEDIUM: 217

### 按问题类型统计
- LABEL_ERROR: 217
- MISSING_FILE: 190
- MISSING_TARGET: 30

## 详细问题列表

### 问题 1
- **文件**: /home/liying/sqlcc/BUILD.bazel
- **行号**: 13
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/sqlcc_server:server_main.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `srcs = ["//src/sqlcc_server:server_main.cpp"],`
- **可自动修复**: 否

### 问题 2
- **文件**: /home/liying/sqlcc/BUILD.bazel
- **行号**: 46
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/isql_network:client_main.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `srcs = ["//src/isql_network:client_main.cpp"],`
- **可自动修复**: 否

### 问题 3
- **文件**: /home/liying/sqlcc/BUILD.bazel
- **行号**: 111
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/utils:logger.cppm"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/utils:logger.cppm",`
- **可自动修复**: 否

### 问题 4
- **文件**: /home/liying/sqlcc/BUILD.bazel
- **行号**: 112
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/utils:logger_module.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//src/utils:logger_module.cpp",`
- **可自动修复**: 否

### 问题 5
- **文件**: /home/liying/sqlcc/BUILD.bazel
- **行号**: 17
- **类型**: MISSING_TARGET
- **严重程度**: HIGH
- **问题描述**: 依赖目标不存在: //src/network:network
- **修复建议**: 检查目标名称或创建缺失的目标
- **可自动修复**: 否

### 问题 6
- **文件**: /home/liying/sqlcc/BUILD.bazel
- **行号**: 13
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/sqlcc_server:server_main.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 7
- **文件**: /home/liying/sqlcc/BUILD.bazel
- **行号**: 46
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/isql_network:client_main.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 8
- **文件**: /home/liying/sqlcc/include/BUILD.bazel
- **行号**: 145
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: config_manager.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 9
- **文件**: /home/liying/sqlcc/include/storage_engine/BUILD.bazel
- **行号**: 37
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 10
- **文件**: /home/liying/sqlcc/include/storage_engine/BUILD.bazel
- **行号**: 64
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 11
- **文件**: /home/liying/sqlcc/include/storage_engine/BUILD.bazel
- **行号**: 91
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 12
- **文件**: /home/liying/sqlcc/include/storage_engine/buffer_pool/BUILD.bazel
- **行号**: 4
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/storage_engine/buffer_pool:buffer_pool_src
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 13
- **文件**: /home/liying/sqlcc/include/storage_engine/table_storage/BUILD.bazel
- **行号**: 13
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 14
- **文件**: /home/liying/sqlcc/include/storage_engine/table_storage/BUILD.bazel
- **行号**: 41
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 15
- **文件**: /home/liying/sqlcc/tests/BUILD.bazel
- **行号**: 22
- **类型**: MISSING_TARGET
- **严重程度**: HIGH
- **问题描述**: 依赖目标不存在: //src/trigger:trigger
- **修复建议**: 检查目标名称或创建缺失的目标
- **可自动修复**: 否

### 问题 16
- **文件**: /home/liying/sqlcc/tests/BUILD.bazel
- **行号**: 10
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: parser_create_table_test.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 17
- **文件**: /home/liying/sqlcc/tests/BUILD.bazel
- **行号**: 36
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: parser_create_table_advanced_test.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 18
- **文件**: /home/liying/sqlcc/tests/BUILD.bazel
- **行号**: 62
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: parser_alter_table_test.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 19
- **文件**: /home/liying/sqlcc/tests/BUILD.bazel
- **行号**: 88
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: parser_alter_table_comprehensive_test.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 20
- **文件**: /home/liying/sqlcc/tests/BUILD.bazel
- **行号**: 114
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: parser_drop_table_test.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 21
- **文件**: /home/liying/sqlcc/tests/BUILD.bazel
- **行号**: 140
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: parser_select_test.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 22
- **文件**: /home/liying/sqlcc/tests/BUILD.bazel
- **行号**: 166
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: parser_insert_test.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 23
- **文件**: /home/liying/sqlcc/tests/BUILD.bazel
- **行号**: 192
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: debug_lexer_test.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 24
- **文件**: /home/liying/sqlcc/tests/BUILD.bazel
- **行号**: 218
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: debug_token_enum_values.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 25
- **文件**: /home/liying/sqlcc/tests/BUILD.bazel
- **行号**: 244
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: debug_keyword_check.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 26
- **文件**: /home/liying/sqlcc/tests/storage_engine/BUILD.bazel
- **行号**: 9
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 27
- **文件**: /home/liying/sqlcc/tests/storage_engine/BUILD.bazel
- **行号**: 22
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 28
- **文件**: /home/liying/sqlcc/tests/storage_engine/BUILD.bazel
- **行号**: 33
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 29
- **文件**: /home/liying/sqlcc/tests/storage_engine/BUILD.bazel
- **行号**: 44
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 30
- **文件**: /home/liying/sqlcc/tests/storage_engine/BUILD.bazel
- **行号**: 55
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 31
- **文件**: /home/liying/sqlcc/tests/storage_engine/BUILD.bazel
- **行号**: 66
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 32
- **文件**: /home/liying/sqlcc/tests/storage_engine/BUILD.bazel
- **行号**: 77
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 33
- **文件**: /home/liying/sqlcc/tests/unit/network/BUILD.bazel
- **行号**: 8
- **类型**: MISSING_TARGET
- **严重程度**: HIGH
- **问题描述**: 依赖目标不存在: //src/network:network
- **修复建议**: 检查目标名称或创建缺失的目标
- **可自动修复**: 否

### 问题 34
- **文件**: /home/liying/sqlcc/tests/unit/storage/BUILD.bazel
- **行号**: 8
- **类型**: MISSING_TARGET
- **严重程度**: HIGH
- **问题描述**: 依赖目标不存在: //src/storage_engine:lazy_writer
- **修复建议**: 检查目标名称或创建缺失的目标
- **可自动修复**: 否

### 问题 35
- **文件**: /home/liying/sqlcc/tests/unit/basic/BUILD.bazel
- **行号**: 13
- **类型**: MISSING_TARGET
- **严重程度**: HIGH
- **问题描述**: 依赖目标不存在: //src/core:sqlcc_core_lib
- **修复建议**: 检查目标名称或创建缺失的目标
- **可自动修复**: 否

### 问题 36
- **文件**: /home/liying/sqlcc/tests/unit/basic/BUILD.bazel
- **行号**: 17
- **类型**: MISSING_TARGET
- **严重程度**: HIGH
- **问题描述**: 依赖目标不存在: //src:execution_context
- **修复建议**: 检查目标名称或创建缺失的目标
- **可自动修复**: 否

### 问题 37
- **文件**: /home/liying/sqlcc/tests/unit/parser/BUILD.bazel
- **行号**: 7
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 38
- **文件**: /home/liying/sqlcc/tests/integration/BUILD.bazel
- **行号**: 11
- **类型**: MISSING_TARGET
- **严重程度**: HIGH
- **问题描述**: 依赖目标不存在: //src/core:sqlcc_core_lib
- **修复建议**: 检查目标名称或创建缺失的目标
- **可自动修复**: 否

### 问题 39
- **文件**: /home/liying/sqlcc/tests/integration/BUILD.bazel
- **行号**: 12
- **类型**: MISSING_TARGET
- **严重程度**: HIGH
- **问题描述**: 依赖目标不存在: //src/core:sqlcc_database_core
- **修复建议**: 检查目标名称或创建缺失的目标
- **可自动修复**: 否

### 问题 40
- **文件**: /home/liying/sqlcc/tests/integration/BUILD.bazel
- **行号**: 13
- **类型**: MISSING_TARGET
- **严重程度**: HIGH
- **问题描述**: 依赖目标不存在: //src/network:network
- **修复建议**: 检查目标名称或创建缺失的目标
- **可自动修复**: 否

### 问题 41
- **文件**: /home/liying/sqlcc/tests/integration/BUILD.bazel
- **行号**: 20
- **类型**: MISSING_TARGET
- **严重程度**: HIGH
- **问题描述**: 依赖目标不存在: //src/trigger:trigger
- **修复建议**: 检查目标名称或创建缺失的目标
- **可自动修复**: 否

### 问题 42
- **文件**: /home/liying/sqlcc/tests/integration/BUILD.bazel
- **行号**: 21
- **类型**: MISSING_TARGET
- **严重程度**: HIGH
- **问题描述**: 依赖目标不存在: //src/sql_executor:sqlcc_executor
- **修复建议**: 检查目标名称或创建缺失的目标
- **可自动修复**: 否

### 问题 43
- **文件**: /home/liying/sqlcc/tests/integration/BUILD.bazel
- **行号**: 22
- **类型**: MISSING_TARGET
- **严重程度**: HIGH
- **问题描述**: 依赖目标不存在: //src:unified_executor
- **修复建议**: 检查目标名称或创建缺失的目标
- **可自动修复**: 否

### 问题 44
- **文件**: /home/liying/sqlcc/tests/stubs/BUILD.bazel
- **行号**: 7
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 45
- **文件**: /home/liying/sqlcc/tests/components/network/BUILD.bazel
- **行号**: 10
- **类型**: MISSING_TARGET
- **严重程度**: HIGH
- **问题描述**: 依赖目标不存在: //src/network:network
- **修复建议**: 检查目标名称或创建缺失的目标
- **可自动修复**: 否

### 问题 46
- **文件**: /home/liying/sqlcc/tests/components/network/BUILD.bazel
- **行号**: 11
- **类型**: MISSING_TARGET
- **严重程度**: HIGH
- **问题描述**: 依赖目标不存在: //src/network:sqlcc_network
- **修复建议**: 检查目标名称或创建缺失的目标
- **可自动修复**: 否

### 问题 47
- **文件**: /home/liying/sqlcc/tests/components/core/BUILD.bazel
- **行号**: 8
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `#     includes = ["//include", "../../../include/core", "../../../include/storage", "../../../src/sql_executor"],`
- **可自动修复**: 否

### 问题 48
- **文件**: /home/liying/sqlcc/tests/components/core/BUILD.bazel
- **行号**: 21
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include", "../../../include/core"],`
- **可自动修复**: 否

### 问题 49
- **文件**: /home/liying/sqlcc/tests/components/core/BUILD.bazel
- **行号**: 34
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include", "../../../include/core"],`
- **可自动修复**: 否

### 问题 50
- **文件**: /home/liying/sqlcc/tests/components/core/BUILD.bazel
- **行号**: 47
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include", "../../../include/core"],`
- **可自动修复**: 否

### 问题 51
- **文件**: /home/liying/sqlcc/tests/components/core/BUILD.bazel
- **行号**: 59
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include", "../../../include/core"],`
- **可自动修复**: 否

### 问题 52
- **文件**: /home/liying/sqlcc/tests/components/core/BUILD.bazel
- **行号**: 11
- **类型**: MISSING_TARGET
- **严重程度**: HIGH
- **问题描述**: 依赖目标不存在: //src/core:sqlcc_core_lib
- **修复建议**: 检查目标名称或创建缺失的目标
- **可自动修复**: 否

### 问题 53
- **文件**: /home/liying/sqlcc/tests/components/core/BUILD.bazel
- **行号**: 12
- **类型**: MISSING_TARGET
- **严重程度**: HIGH
- **问题描述**: 依赖目标不存在: //src/sql_executor:sqlcc_executor
- **修复建议**: 检查目标名称或创建缺失的目标
- **可自动修复**: 否

### 问题 54
- **文件**: /home/liying/sqlcc/tests/components/core/BUILD.bazel
- **行号**: 13
- **类型**: MISSING_TARGET
- **严重程度**: HIGH
- **问题描述**: 依赖目标不存在: //src:execution_engine
- **修复建议**: 检查目标名称或创建缺失的目标
- **可自动修复**: 否

### 问题 55
- **文件**: /home/liying/sqlcc/tests/components/core/BUILD.bazel
- **行号**: 14
- **类型**: MISSING_TARGET
- **严重程度**: HIGH
- **问题描述**: 依赖目标不存在: //src:unified_executor
- **修复建议**: 检查目标名称或创建缺失的目标
- **可自动修复**: 否

### 问题 56
- **文件**: /home/liying/sqlcc/tests/components/core/BUILD.bazel
- **行号**: 7
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: constraint_validation_test.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 57
- **文件**: /home/liying/sqlcc/tests/components/parser/BUILD.bazel
- **行号**: 84
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 58
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **行号**: 9
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 59
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **行号**: 23
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 60
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **行号**: 36
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 61
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **行号**: 49
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 62
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **行号**: 62
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 63
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **行号**: 75
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 64
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **行号**: 88
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 65
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **行号**: 101
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 66
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **行号**: 114
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 67
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **行号**: 127
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 68
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **行号**: 140
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 69
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **行号**: 153
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 70
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **行号**: 166
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 71
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **行号**: 179
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 72
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **行号**: 192
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 73
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **行号**: 205
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 74
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **行号**: 218
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 75
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **行号**: 231
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 76
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **行号**: 244
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 77
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **行号**: 257
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 78
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **行号**: 270
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 79
- **文件**: /home/liying/sqlcc/tests/sql_parser/BUILD.bazel
- **行号**: 346
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 80
- **文件**: /home/liying/sqlcc/tests/performance/BUILD.bazel
- **行号**: 16
- **类型**: MISSING_TARGET
- **严重程度**: HIGH
- **问题描述**: 依赖目标不存在: //src/core:sqlcc_core_lib
- **修复建议**: 检查目标名称或创建缺失的目标
- **可自动修复**: 否

### 问题 81
- **文件**: /home/liying/sqlcc/tests/performance/BUILD.bazel
- **行号**: 17
- **类型**: MISSING_TARGET
- **严重程度**: HIGH
- **问题描述**: 依赖目标不存在: //src/sql_executor:sqlcc_executor
- **修复建议**: 检查目标名称或创建缺失的目标
- **可自动修复**: 否

### 问题 82
- **文件**: /home/liying/sqlcc/tests/performance/BUILD.bazel
- **行号**: 18
- **类型**: MISSING_TARGET
- **严重程度**: HIGH
- **问题描述**: 依赖目标不存在: //src:sqlcc_core
- **修复建议**: 检查目标名称或创建缺失的目标
- **可自动修复**: 否

### 问题 83
- **文件**: /home/liying/sqlcc/tests/sql_executor/BUILD.bazel
- **行号**: 7
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 84
- **文件**: /home/liying/sqlcc/tests/sql_executor/BUILD.bazel
- **行号**: 19
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 85
- **文件**: /home/liying/sqlcc/tests/sql_executor/BUILD.bazel
- **行号**: 32
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 86
- **文件**: /home/liying/sqlcc/tests/sql_executor/BUILD.bazel
- **行号**: 43
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 87
- **文件**: /home/liying/sqlcc/tests/sql_executor/BUILD.bazel
- **行号**: 54
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 88
- **文件**: /home/liying/sqlcc/tests/sql_executor/BUILD.bazel
- **行号**: 65
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 89
- **文件**: /home/liying/sqlcc/tests/sql_executor/BUILD.bazel
- **行号**: 77
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 90
- **文件**: /home/liying/sqlcc/tests/sql_executor/BUILD.bazel
- **行号**: 11
- **类型**: MISSING_TARGET
- **严重程度**: HIGH
- **问题描述**: 依赖目标不存在: //src/sql_executor:sqlcc_executor
- **修复建议**: 检查目标名称或创建缺失的目标
- **可自动修复**: 否

### 问题 91
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 12
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/logger:logger.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//src/logger:logger.cpp",`
- **可自动修复**: 否

### 问题 92
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 13
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/logger:logger_module_impl.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//src/logger:logger_module_impl.cpp",`
- **可自动修复**: 否

### 问题 93
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 15
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/utils:logger.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `hdrs = ["//include/utils:logger.h"],`
- **可自动修复**: 否

### 问题 94
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 39
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/utils:config_snapshot.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//src/utils:config_snapshot.cpp",`
- **可自动修复**: 否

### 问题 95
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 40
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/utils:config_lifecycle.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//src/utils:config_lifecycle.cpp",`
- **可自动修复**: 否

### 问题 96
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 41
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/utils:smart_config_manager.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//src/utils:smart_config_manager.cpp",`
- **可自动修复**: 否

### 问题 97
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 44
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/utils:config_snapshot.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/utils:config_snapshot.h",`
- **可自动修复**: 否

### 问题 98
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 45
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/utils:config_lifecycle.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/utils:config_lifecycle.h",`
- **可自动修复**: 否

### 问题 99
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 46
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/utils:smart_config_manager.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/utils:smart_config_manager.h",`
- **可自动修复**: 否

### 问题 100
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 47
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/utils:ssl_wrapper.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/utils:ssl_wrapper.h",`
- **可自动修复**: 否

### 问题 101
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 80
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/core:user_manager.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//src/core:user_manager.cpp",`
- **可自动修复**: 否

### 问题 102
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 81
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/core:execution_context.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//src/core:execution_context.cpp",`
- **可自动修复**: 否

### 问题 103
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 82
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/core:unified_executor.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//src/core:unified_executor.cpp",`
- **可自动修复**: 否

### 问题 104
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 85
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/core:user_manager.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/core:user_manager.h",`
- **可自动修复**: 否

### 问题 105
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 86
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/core:execution_context.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/core:execution_context.h",`
- **可自动修复**: 否

### 问题 106
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 87
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/core:unified_executor.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/core:unified_executor.h",`
- **可自动修复**: 否

### 问题 107
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 115
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/core:user_manager.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `srcs = ["//src/core:user_manager.cpp"],`
- **可自动修复**: 否

### 问题 108
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 116
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/core:user_manager.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `hdrs = ["//include/core:user_manager.h"],`
- **可自动修复**: 否

### 问题 109
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 146
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/storage_engine:storage_engine.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//src/storage_engine:storage_engine.cpp",`
- **可自动修复**: 否

### 问题 110
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 147
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/storage_engine:b_plus_tree.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//src/storage_engine:b_plus_tree.cpp",`
- **可自动修复**: 否

### 问题 111
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 148
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/storage_engine:buffer_pool.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//src/storage_engine:buffer_pool.cpp",`
- **可自动修复**: 否

### 问题 112
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 149
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/storage_engine:wal_buffer.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//src/storage_engine:wal_buffer.cpp",`
- **可自动修复**: 否

### 问题 113
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 150
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/storage_engine:wal_writer.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//src/storage_engine:wal_writer.cpp",`
- **可自动修复**: 否

### 问题 114
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 151
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/storage_engine:checkpoint.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//src/storage_engine:checkpoint.cpp",`
- **可自动修复**: 否

### 问题 115
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 152
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/storage_engine:page_allocator.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//src/storage_engine:page_allocator.cpp",`
- **可自动修复**: 否

### 问题 116
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 153
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/storage_engine:concurrent_access_validator.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//src/storage_engine:concurrent_access_validator.cpp",`
- **可自动修复**: 否

### 问题 117
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 154
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/storage_engine:disk_error_handler.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//src/storage_engine:disk_error_handler.cpp",`
- **可自动修复**: 否

### 问题 118
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 155
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/storage_engine:partition_manager.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//src/storage_engine:partition_manager.cpp",`
- **可自动修复**: 否

### 问题 119
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 156
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/storage_engine:record_boundary_validator.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//src/storage_engine:record_boundary_validator.cpp",`
- **可自动修复**: 否

### 问题 120
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 157
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/storage_engine:replace_strategy.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//src/storage_engine:replace_strategy.cpp",`
- **可自动修复**: 否

### 问题 121
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 158
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/storage_engine:cache_consistency_manager.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//src/storage_engine:cache_consistency_manager.cpp",`
- **可自动修复**: 否

### 问题 122
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 159
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/storage_engine:advanced_lock_manager.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//src/storage_engine:advanced_lock_manager.cpp",`
- **可自动修复**: 否

### 问题 123
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 160
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/storage_engine:data_integrity_validator.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//src/storage_engine:data_integrity_validator.cpp",`
- **可自动修复**: 否

### 问题 124
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 161
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/storage_engine:buffer_pool_sharded.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//src/storage_engine:buffer_pool_sharded.cpp",`
- **可自动修复**: 否

### 问题 125
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 162
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/storage_engine:concurrency_control.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//src/storage_engine:concurrency_control.cpp",`
- **可自动修复**: 否

### 问题 126
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 163
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/storage_engine:table_storage_complete.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//src/storage_engine:table_storage_complete.cpp",`
- **可自动修复**: 否

### 问题 127
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 167
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/storage:b_plus_tree.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/storage:b_plus_tree.h",`
- **可自动修复**: 否

### 问题 128
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 168
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/storage:buffer_pool.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/storage:buffer_pool.h",`
- **可自动修复**: 否

### 问题 129
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 169
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/storage:wal_buffer.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/storage:wal_buffer.h",`
- **可自动修复**: 否

### 问题 130
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 170
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/storage:wal_writer.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/storage:wal_writer.h",`
- **可自动修复**: 否

### 问题 131
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 171
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/storage:checkpoint.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/storage:checkpoint.h",`
- **可自动修复**: 否

### 问题 132
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 172
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/storage:page_allocator.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/storage:page_allocator.h",`
- **可自动修复**: 否

### 问题 133
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 173
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/storage:concurrent_access_validator.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/storage:concurrent_access_validator.h",`
- **可自动修复**: 否

### 问题 134
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 174
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/storage:disk_error_handler.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/storage:disk_error_handler.h",`
- **可自动修复**: 否

### 问题 135
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 175
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/storage:partition_manager.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/storage:partition_manager.h",`
- **可自动修复**: 否

### 问题 136
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 176
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/storage:record_boundary_validator.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/storage:record_boundary_validator.h",`
- **可自动修复**: 否

### 问题 137
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 177
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/storage:replace_strategy.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/storage:replace_strategy.h",`
- **可自动修复**: 否

### 问题 138
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 178
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/storage:cache_consistency_manager.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/storage:cache_consistency_manager.h",`
- **可自动修复**: 否

### 问题 139
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 179
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/storage:advanced_lock_manager.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/storage:advanced_lock_manager.h",`
- **可自动修复**: 否

### 问题 140
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 180
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/storage:data_integrity_validator.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/storage:data_integrity_validator.h",`
- **可自动修复**: 否

### 问题 141
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 181
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/storage:buffer_pool_sharded.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/storage:buffer_pool_sharded.h",`
- **可自动修复**: 否

### 问题 142
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 182
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/storage:concurrency_control.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/storage:concurrency_control.h",`
- **可自动修复**: 否

### 问题 143
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 183
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/storage:b_plus_tree_nodes.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/storage:b_plus_tree_nodes.h",`
- **可自动修复**: 否

### 问题 144
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 184
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/storage:buffer_pool_fixed.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/storage:buffer_pool_fixed.h",`
- **可自动修复**: 否

### 问题 145
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 185
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/storage:buffer_pool_v2.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/storage:buffer_pool_v2.h",`
- **可自动修复**: 否

### 问题 146
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 186
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/storage:disk_manager.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/storage:disk_manager.h",`
- **可自动修复**: 否

### 问题 147
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 213
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/storage_engine:index_manager/index_manager.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//src/storage_engine:index_manager/index_manager.cpp",`
- **可自动修复**: 否

### 问题 148
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 214
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/storage_engine:index_manager/index_manager_smart_ptr_enhancement.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//src/storage_engine:index_manager/index_manager_smart_ptr_enhancement.cpp",`
- **可自动修复**: 否

### 问题 149
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 217
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/storage:index_manager.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/storage:index_manager.h",`
- **可自动修复**: 否

### 问题 150
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 247
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/sql_parser:parser.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//src/sql_parser:parser.cpp",`
- **可自动修复**: 否

### 问题 151
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 248
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/sql_parser:lexer.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//src/sql_parser:lexer.cpp",`
- **可自动修复**: 否

### 问题 152
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 249
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/sql_parser:token.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//src/sql_parser:token.cpp",`
- **可自动修复**: 否

### 问题 153
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 250
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/sql_parser:ast_node.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//src/sql_parser:ast_node.cpp",`
- **可自动修复**: 否

### 问题 154
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 251
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/sql_parser:ast_nodes.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//src/sql_parser:ast_nodes.cpp",`
- **可自动修复**: 否

### 问题 155
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 252
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/sql_parser:constraint.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//src/sql_parser:constraint.cpp",`
- **可自动修复**: 否

### 问题 156
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 253
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/sql_parser:set_operation.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//src/sql_parser:set_operation.cpp",`
- **可自动修复**: 否

### 问题 157
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 254
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/sql_parser:window_function.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//src/sql_parser:window_function.cpp",`
- **可自动修复**: 否

### 问题 158
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 255
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/sql_parser:recursive_query.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//src/sql_parser:recursive_query.cpp",`
- **可自动修复**: 否

### 问题 159
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 256
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/sql_parser:decimal.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//src/sql_parser:decimal.cpp",`
- **可自动修复**: 否

### 问题 160
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 257
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/sql_parser:json.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//src/sql_parser:json.cpp",`
- **可自动修复**: 否

### 问题 161
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 258
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/sql_parser:datetime.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//src/sql_parser:datetime.cpp",`
- **可自动修复**: 否

### 问题 162
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 259
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/sql_parser:function_ast.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//src/sql_parser:function_ast.cpp",`
- **可自动修复**: 否

### 问题 163
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 262
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/sql_parser:parser.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/sql_parser:parser.h",`
- **可自动修复**: 否

### 问题 164
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 263
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/sql_parser:lexer.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/sql_parser:lexer.h",`
- **可自动修复**: 否

### 问题 165
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 264
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/sql_parser:token.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/sql_parser:token.h",`
- **可自动修复**: 否

### 问题 166
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 265
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/sql_parser:ast_node.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/sql_parser:ast_node.h",`
- **可自动修复**: 否

### 问题 167
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 266
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/sql_parser:ast_nodes.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/sql_parser:ast_nodes.h",`
- **可自动修复**: 否

### 问题 168
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 267
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/sql_parser:constraint.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/sql_parser:constraint.h",`
- **可自动修复**: 否

### 问题 169
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 268
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/sql_parser:set_operation.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/sql_parser:set_operation.h",`
- **可自动修复**: 否

### 问题 170
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 269
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/sql_parser:window_function.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/sql_parser:window_function.h",`
- **可自动修复**: 否

### 问题 171
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 270
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/sql_parser:recursive_query.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/sql_parser:recursive_query.h",`
- **可自动修复**: 否

### 问题 172
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 271
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/sql_parser:decimal.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/sql_parser:decimal.h",`
- **可自动修复**: 否

### 问题 173
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 272
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/sql_parser:json.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/sql_parser:json.h",`
- **可自动修复**: 否

### 问题 174
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 273
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/sql_parser:datetime.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/sql_parser:datetime.h",`
- **可自动修复**: 否

### 问题 175
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 274
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/sql_parser:function_ast.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/sql_parser:function_ast.h",`
- **可自动修复**: 否

### 问题 176
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 275
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/sql_parser:node_visitor.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/sql_parser:node_visitor.h",`
- **可自动修复**: 否

### 问题 177
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 276
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/sql_parser:load_data_ast.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/sql_parser:load_data_ast.h",`
- **可自动修复**: 否

### 问题 178
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 303
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/execution:function_executor.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//src/execution:function_executor.cpp",`
- **可自动修复**: 否

### 问题 179
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 304
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/execution:set_operation_executor.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//src/execution:set_operation_executor.cpp",`
- **可自动修复**: 否

### 问题 180
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 305
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/execution:window_function_executor.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//src/execution:window_function_executor.cpp",`
- **可自动修复**: 否

### 问题 181
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 306
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/execution:recursive_query_executor.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//src/execution:recursive_query_executor.cpp",`
- **可自动修复**: 否

### 问题 182
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 307
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/execution:load_data_executor.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//src/execution:load_data_executor.cpp",`
- **可自动修复**: 否

### 问题 183
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 310
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/execution:function_executor.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/execution:function_executor.h",`
- **可自动修复**: 否

### 问题 184
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 311
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/execution:set_operation_executor.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/execution:set_operation_executor.h",`
- **可自动修复**: 否

### 问题 185
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 312
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/execution:window_function_executor.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/execution:window_function_executor.h",`
- **可自动修复**: 否

### 问题 186
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 313
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/execution:recursive_query_executor.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/execution:recursive_query_executor.h",`
- **可自动修复**: 否

### 问题 187
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 314
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/execution:load_data_executor.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/execution:load_data_executor.h",`
- **可自动修复**: 否

### 问题 188
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 315
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/execution:join_executor.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/execution:join_executor.h",`
- **可自动修复**: 否

### 问题 189
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 316
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/execution:procedure_trigger_task.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/execution:procedure_trigger_task.h",`
- **可自动修复**: 否

### 问题 190
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 346
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/sql_executor:sql_executor.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `srcs = ["//src/sql_executor:sql_executor.cpp"],`
- **可自动修复**: 否

### 问题 191
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 379
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/network:network.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//src/network:network.cpp",`
- **可自动修复**: 否

### 问题 192
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 380
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/network:connection_state_machine.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//src/network:connection_state_machine.cpp",`
- **可自动修复**: 否

### 问题 193
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 381
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/network:encryption.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//src/network:encryption.cpp",`
- **可自动修复**: 否

### 问题 194
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 382
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/network:data_transmission_validator.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//src/network:data_transmission_validator.cpp",`
- **可自动修复**: 否

### 问题 195
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 383
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/network:mysql_protocol.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//src/network:mysql_protocol.cpp",`
- **可自动修复**: 否

### 问题 196
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 386
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/network:network.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/network:network.h",`
- **可自动修复**: 否

### 问题 197
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 387
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/network:connection_state.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/network:connection_state.h",`
- **可自动修复**: 否

### 问题 198
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 388
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/network:connection_state_machine.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/network:connection_state_machine.h",`
- **可自动修复**: 否

### 问题 199
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 389
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/network:encryption.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/network:encryption.h",`
- **可自动修复**: 否

### 问题 200
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 390
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/network:data_transmission_validator.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/network:data_transmission_validator.h",`
- **可自动修复**: 否

### 问题 201
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 391
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/network:mysql_protocol.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/network:mysql_protocol.h",`
- **可自动修复**: 否

### 问题 202
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 419
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/config_manager:config_manager.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `srcs = ["//src/config_manager:config_manager.cpp"],`
- **可自动修复**: 否

### 问题 203
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 420
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/config_manager:config_manager.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `hdrs = ["//include/config_manager:config_manager.h"],`
- **可自动修复**: 否

### 问题 204
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 454
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/procedure:procedure_parser.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//src/procedure:procedure_parser.cpp",`
- **可自动修复**: 否

### 问题 205
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 455
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/procedure:procedure_vm.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//src/procedure:procedure_vm.cpp",`
- **可自动修复**: 否

### 问题 206
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 456
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/procedure:procedure_trigger_executor.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//src/procedure:procedure_trigger_executor.cpp",`
- **可自动修复**: 否

### 问题 207
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 459
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/procedure:procedure_parser.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/procedure:procedure_parser.h",`
- **可自动修复**: 否

### 问题 208
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 460
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/procedure:procedure_vm.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/procedure:procedure_vm.h",`
- **可自动修复**: 否

### 问题 209
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 461
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/procedure:procedure_trigger_executor.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/procedure:procedure_trigger_executor.h",`
- **可自动修复**: 否

### 问题 210
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 489
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/trigger:trigger_manager.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `srcs = ["//src/trigger:trigger_manager.cpp"],`
- **可自动修复**: 否

### 问题 211
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 490
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/trigger:trigger_manager.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `hdrs = ["//include/trigger:trigger_manager.h"],`
- **可自动修复**: 否

### 问题 212
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 519
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/transaction:savepoint_manager.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `srcs = ["//src/transaction:savepoint_manager.cpp"],`
- **可自动修复**: 否

### 问题 213
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 520
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/transaction:savepoint_manager.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `hdrs = ["//include/transaction:savepoint_manager.h"],`
- **可自动修复**: 否

### 问题 214
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 549
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/types:domain_manager.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `srcs = ["//src/types:domain_manager.cpp"],`
- **可自动修复**: 否

### 问题 215
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 550
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/types:domain_manager.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `hdrs = ["//include/types:domain_manager.h"],`
- **可自动修复**: 否

### 问题 216
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 582
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//src/security:memory_monitor.cpp"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//src/security:memory_monitor.cpp",`
- **可自动修复**: 否

### 问题 217
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 585
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/security:memory_monitor.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/security:memory_monitor.h",`
- **可自动修复**: 否

### 问题 218
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 12
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/logger:logger.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 219
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 13
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/logger:logger_module_impl.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 220
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 15
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/utils:logger.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 221
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 39
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/utils:config_snapshot.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 222
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 40
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/utils:config_lifecycle.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 223
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 41
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/utils:smart_config_manager.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 224
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 44
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/utils:config_snapshot.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 225
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 45
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/utils:config_lifecycle.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 226
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 46
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/utils:smart_config_manager.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 227
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 47
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/utils:ssl_wrapper.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 228
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 80
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/core:user_manager.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 229
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 81
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/core:execution_context.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 230
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 82
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/core:unified_executor.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 231
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 85
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/core:user_manager.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 232
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 86
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/core:execution_context.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 233
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 87
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/core:unified_executor.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 234
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 80
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/core:user_manager.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 235
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 85
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/core:user_manager.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 236
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 146
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/storage_engine:storage_engine.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 237
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 147
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/storage_engine:b_plus_tree.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 238
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 148
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/storage_engine:buffer_pool.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 239
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 149
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/storage_engine:wal_buffer.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 240
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 150
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/storage_engine:wal_writer.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 241
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 151
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/storage_engine:checkpoint.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 242
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 152
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/storage_engine:page_allocator.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 243
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 153
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/storage_engine:concurrent_access_validator.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 244
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 154
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/storage_engine:disk_error_handler.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 245
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 155
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/storage_engine:partition_manager.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 246
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 156
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/storage_engine:record_boundary_validator.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 247
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 157
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/storage_engine:replace_strategy.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 248
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 158
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/storage_engine:cache_consistency_manager.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 249
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 159
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/storage_engine:advanced_lock_manager.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 250
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 160
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/storage_engine:data_integrity_validator.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 251
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 161
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/storage_engine:buffer_pool_sharded.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 252
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 162
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/storage_engine:concurrency_control.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 253
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 163
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/storage_engine:table_storage_complete.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 254
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 166
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include:storage_engine
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 255
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 167
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/storage:b_plus_tree.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 256
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 168
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/storage:buffer_pool.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 257
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 169
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/storage:wal_buffer.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 258
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 170
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/storage:wal_writer.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 259
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 171
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/storage:checkpoint.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 260
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 172
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/storage:page_allocator.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 261
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 173
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/storage:concurrent_access_validator.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 262
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 174
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/storage:disk_error_handler.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 263
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 175
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/storage:partition_manager.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 264
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 176
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/storage:record_boundary_validator.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 265
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 177
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/storage:replace_strategy.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 266
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 178
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/storage:cache_consistency_manager.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 267
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 179
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/storage:advanced_lock_manager.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 268
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 180
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/storage:data_integrity_validator.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 269
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 181
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/storage:buffer_pool_sharded.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 270
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 182
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/storage:concurrency_control.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 271
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 183
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/storage:b_plus_tree_nodes.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 272
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 184
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/storage:buffer_pool_fixed.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 273
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 185
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/storage:buffer_pool_v2.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 274
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 186
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/storage:disk_manager.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 275
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 213
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/storage_engine:index_manager/index_manager.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 276
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 214
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/storage_engine:index_manager/index_manager_smart_ptr_enhancement.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 277
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 217
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/storage:index_manager.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 278
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 247
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/sql_parser:parser.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 279
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 248
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/sql_parser:lexer.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 280
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 249
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/sql_parser:token.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 281
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 250
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/sql_parser:ast_node.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 282
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 251
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/sql_parser:ast_nodes.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 283
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 252
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/sql_parser:constraint.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 284
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 253
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/sql_parser:set_operation.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 285
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 254
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/sql_parser:window_function.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 286
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 255
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/sql_parser:recursive_query.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 287
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 256
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/sql_parser:decimal.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 288
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 257
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/sql_parser:json.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 289
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 258
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/sql_parser:datetime.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 290
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 259
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/sql_parser:function_ast.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 291
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 262
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/sql_parser:parser.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 292
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 263
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/sql_parser:lexer.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 293
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 264
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/sql_parser:token.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 294
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 265
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/sql_parser:ast_node.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 295
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 266
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/sql_parser:ast_nodes.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 296
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 267
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/sql_parser:constraint.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 297
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 268
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/sql_parser:set_operation.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 298
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 269
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/sql_parser:window_function.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 299
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 270
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/sql_parser:recursive_query.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 300
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 271
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/sql_parser:decimal.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 301
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 272
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/sql_parser:json.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 302
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 273
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/sql_parser:datetime.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 303
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 274
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/sql_parser:function_ast.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 304
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 275
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/sql_parser:node_visitor.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 305
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 276
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/sql_parser:load_data_ast.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 306
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 303
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/execution:function_executor.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 307
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 304
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/execution:set_operation_executor.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 308
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 305
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/execution:window_function_executor.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 309
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 306
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/execution:recursive_query_executor.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 310
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 307
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/execution:load_data_executor.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 311
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 310
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/execution:function_executor.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 312
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 311
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/execution:set_operation_executor.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 313
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 312
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/execution:window_function_executor.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 314
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 313
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/execution:recursive_query_executor.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 315
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 314
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/execution:load_data_executor.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 316
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 315
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/execution:join_executor.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 317
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 316
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/execution:procedure_trigger_task.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 318
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 346
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/sql_executor:sql_executor.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 319
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 347
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/sql_executor:headers
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 320
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 379
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/network:network.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 321
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 380
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/network:connection_state_machine.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 322
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 381
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/network:encryption.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 323
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 382
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/network:data_transmission_validator.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 324
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 383
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/network:mysql_protocol.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 325
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 386
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/network:network.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 326
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 387
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/network:connection_state.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 327
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 388
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/network:connection_state_machine.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 328
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 389
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/network:encryption.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 329
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 390
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/network:data_transmission_validator.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 330
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 391
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/network:mysql_protocol.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 331
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 419
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/config_manager:config_manager.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 332
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 420
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/config_manager:config_manager.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 333
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 454
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/procedure:procedure_parser.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 334
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 455
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/procedure:procedure_vm.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 335
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 456
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/procedure:procedure_trigger_executor.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 336
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 459
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/procedure:procedure_parser.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 337
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 460
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/procedure:procedure_vm.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 338
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 461
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/procedure:procedure_trigger_executor.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 339
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 489
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/trigger:trigger_manager.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 340
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 490
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/trigger:trigger_manager.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 341
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 519
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/transaction:savepoint_manager.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 342
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 520
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/transaction:savepoint_manager.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 343
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 549
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/types:domain_manager.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 344
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 550
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/types:domain_manager.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 345
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 582
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //src/security:memory_monitor.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 346
- **文件**: /home/liying/sqlcc/src/BUILD.bazel
- **行号**: 585
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/security:memory_monitor.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 347
- **文件**: /home/liying/sqlcc/src/storage_engine/BUILD.bazel
- **行号**: 83
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 348
- **文件**: /home/liying/sqlcc/src/storage_engine/BUILD.bazel
- **行号**: 115
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 349
- **文件**: /home/liying/sqlcc/src/storage_engine/BUILD.bazel
- **行号**: 144
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 350
- **文件**: /home/liying/sqlcc/src/storage_engine/BUILD.bazel
- **行号**: 37
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/storage:headers
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 351
- **文件**: /home/liying/sqlcc/src/storage_engine/BUILD.bazel
- **行号**: 38
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/storage_engine:b_plus_tree_node
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 352
- **文件**: /home/liying/sqlcc/src/storage_engine/BUILD.bazel
- **行号**: 39
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/storage_engine:b_plus_tree_internal_node
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 353
- **文件**: /home/liying/sqlcc/src/storage_engine/BUILD.bazel
- **行号**: 40
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/storage_engine:b_plus_tree_leaf_node
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 354
- **文件**: /home/liying/sqlcc/src/storage_engine/BUILD.bazel
- **行号**: 41
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/storage_engine:b_plus_tree_index
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 355
- **文件**: /home/liying/sqlcc/src/storage_engine/BUILD.bazel
- **行号**: 78
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: storage_engine_test.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 356
- **文件**: /home/liying/sqlcc/src/storage_engine/BUILD.bazel
- **行号**: 79
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: b_plus_tree_test.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 357
- **文件**: /home/liying/sqlcc/src/storage_engine/BUILD.bazel
- **行号**: 80
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: table_storage_test.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 358
- **文件**: /home/liying/sqlcc/src/storage_engine/BUILD.bazel
- **行号**: 81
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: partition_manager_test.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 359
- **文件**: /home/liying/sqlcc/src/storage_engine/BUILD.bazel
- **行号**: 113
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/storage:partition_manager
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 360
- **文件**: /home/liying/sqlcc/src/storage_engine/BUILD.bazel
- **行号**: 81
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: partition_manager_test.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 361
- **文件**: /home/liying/sqlcc/src/storage_engine/buffer_pool/BUILD.bazel
- **行号**: 8
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/storage_engine/buffer_pool:lru_manager.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/storage_engine/buffer_pool:lru_manager.h",`
- **可自动修复**: 否

### 问题 362
- **文件**: /home/liying/sqlcc/src/storage_engine/buffer_pool/BUILD.bazel
- **行号**: 9
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/storage_engine/buffer_pool:statistics_collector.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/storage_engine/buffer_pool:statistics_collector.h",`
- **可自动修复**: 否

### 问题 363
- **文件**: /home/liying/sqlcc/src/storage_engine/buffer_pool/BUILD.bazel
- **行号**: 8
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/storage_engine/buffer_pool:lru_manager.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 364
- **文件**: /home/liying/sqlcc/src/storage_engine/buffer_pool/BUILD.bazel
- **行号**: 9
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/storage_engine/buffer_pool:statistics_collector.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 365
- **文件**: /home/liying/sqlcc/src/isql_network/BUILD.bazel
- **行号**: 10
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 366
- **文件**: /home/liying/sqlcc/src/isql_network/BUILD.bazel
- **行号**: 13
- **类型**: MISSING_TARGET
- **严重程度**: HIGH
- **问题描述**: 依赖目标不存在: //src/network:network
- **修复建议**: 检查目标名称或创建缺失的目标
- **可自动修复**: 否

### 问题 367
- **文件**: /home/liying/sqlcc/src/network/BUILD.bazel
- **行号**: 4
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/network:session.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `hdrs = ["//include/network:session.h"],`
- **可自动修复**: 否

### 问题 368
- **文件**: /home/liying/sqlcc/src/network/BUILD.bazel
- **行号**: 14
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/network:session_manager.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `hdrs = ["//include/network:session_manager.h"],`
- **可自动修复**: 否

### 问题 369
- **文件**: /home/liying/sqlcc/src/network/BUILD.bazel
- **行号**: 4
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/network:session.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 370
- **文件**: /home/liying/sqlcc/src/network/BUILD.bazel
- **行号**: 14
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/network:session_manager.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 371
- **文件**: /home/liying/sqlcc/src/sqlcc_server/BUILD.bazel
- **行号**: 19
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include", "../../include/core", "../../include/storage"],`
- **可自动修复**: 否

### 问题 372
- **文件**: /home/liying/sqlcc/src/sqlcc_server/BUILD.bazel
- **行号**: 10
- **类型**: MISSING_TARGET
- **严重程度**: HIGH
- **问题描述**: 依赖目标不存在: //src/network:network
- **修复建议**: 检查目标名称或创建缺失的目标
- **可自动修复**: 否

### 问题 373
- **文件**: /home/liying/sqlcc/src/sqlcc_server/BUILD.bazel
- **行号**: 11
- **类型**: MISSING_TARGET
- **严重程度**: HIGH
- **问题描述**: 依赖目标不存在: //src/network:sqlcc_network
- **修复建议**: 检查目标名称或创建缺失的目标
- **可自动修复**: 否

### 问题 374
- **文件**: /home/liying/sqlcc/src/sqlcc_server/BUILD.bazel
- **行号**: 12
- **类型**: MISSING_TARGET
- **严重程度**: HIGH
- **问题描述**: 依赖目标不存在: //src/storage_engine:index_manager
- **修复建议**: 检查目标名称或创建缺失的目标
- **可自动修复**: 否

### 问题 375
- **文件**: /home/liying/sqlcc/src/sqlcc_server/BUILD.bazel
- **行号**: 16
- **类型**: MISSING_TARGET
- **严重程度**: HIGH
- **问题描述**: 依赖目标不存在: //src/sql_executor:sqlcc_executor
- **修复建议**: 检查目标名称或创建缺失的目标
- **可自动修复**: 否

### 问题 376
- **文件**: /home/liying/sqlcc/src/sqlcc_server/BUILD.bazel
- **行号**: 17
- **类型**: MISSING_TARGET
- **严重程度**: HIGH
- **问题描述**: 依赖目标不存在: //src:unified_executor
- **修复建议**: 检查目标名称或创建缺失的目标
- **可自动修复**: 否

### 问题 377
- **文件**: /home/liying/sqlcc/src/logger/BUILD.bazel
- **行号**: 22
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/utils:logger.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `hdrs = ["//include/utils:logger.h"],`
- **可自动修复**: 否

### 问题 378
- **文件**: /home/liying/sqlcc/src/logger/BUILD.bazel
- **行号**: 23
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 379
- **文件**: /home/liying/sqlcc/src/logger/BUILD.bazel
- **行号**: 22
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/utils:logger.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 380
- **文件**: /home/liying/sqlcc/src/core/BUILD.bazel
- **行号**: 20
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/core:user_manager.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/core:user_manager.h",`
- **可自动修复**: 否

### 问题 381
- **文件**: /home/liying/sqlcc/src/core/BUILD.bazel
- **行号**: 21
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/core:permission_validator.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/core:permission_validator.h",`
- **可自动修复**: 否

### 问题 382
- **文件**: /home/liying/sqlcc/src/core/BUILD.bazel
- **行号**: 94
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/core:sql_executor_interface.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/core:sql_executor_interface.h",`
- **可自动修复**: 否

### 问题 383
- **文件**: /home/liying/sqlcc/src/core/BUILD.bazel
- **行号**: 20
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/core:user_manager.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 384
- **文件**: /home/liying/sqlcc/src/core/BUILD.bazel
- **行号**: 21
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/core:permission_validator.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 385
- **文件**: /home/liying/sqlcc/src/core/BUILD.bazel
- **行号**: 22
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/error_handler:error_handler
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 386
- **文件**: /home/liying/sqlcc/src/core/BUILD.bazel
- **行号**: 23
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/exception:exception
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 387
- **文件**: /home/liying/sqlcc/src/core/BUILD.bazel
- **行号**: 61
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/database_manager:database_manager
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 388
- **文件**: /home/liying/sqlcc/src/core/BUILD.bazel
- **行号**: 94
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/core:sql_executor_interface.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 389
- **文件**: /home/liying/sqlcc/src/transaction/BUILD.bazel
- **行号**: 16
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 390
- **文件**: /home/liying/sqlcc/src/transaction/BUILD.bazel
- **行号**: 46
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 391
- **文件**: /home/liying/sqlcc/src/transaction/BUILD.bazel
- **行号**: 14
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/transaction:headers
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 392
- **文件**: /home/liying/sqlcc/src/transaction/BUILD.bazel
- **行号**: 43
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: savepoint_manager_test.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 393
- **文件**: /home/liying/sqlcc/src/transaction/BUILD.bazel
- **行号**: 44
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: transaction_test.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 394
- **文件**: /home/liying/sqlcc/src/config_manager/BUILD.bazel
- **行号**: 11
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 395
- **文件**: /home/liying/sqlcc/src/config_manager/BUILD.bazel
- **行号**: 8
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include:config_manager_headers
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 396
- **文件**: /home/liying/sqlcc/src/utils/BUILD.bazel
- **行号**: 16
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/utils:config_manager.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/utils:config_manager.h",`
- **可自动修复**: 否

### 问题 397
- **文件**: /home/liying/sqlcc/src/utils/BUILD.bazel
- **行号**: 17
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/utils:config_snapshot.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/utils:config_snapshot.h",`
- **可自动修复**: 否

### 问题 398
- **文件**: /home/liying/sqlcc/src/utils/BUILD.bazel
- **行号**: 18
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/utils:config_lifecycle.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/utils:config_lifecycle.h",`
- **可自动修复**: 否

### 问题 399
- **文件**: /home/liying/sqlcc/src/utils/BUILD.bazel
- **行号**: 19
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/utils:smart_config_manager.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/utils:smart_config_manager.h",`
- **可自动修复**: 否

### 问题 400
- **文件**: /home/liying/sqlcc/src/utils/BUILD.bazel
- **行号**: 20
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/utils:ssl_wrapper.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include/utils:ssl_wrapper.h",`
- **可自动修复**: 否

### 问题 401
- **文件**: /home/liying/sqlcc/src/utils/BUILD.bazel
- **行号**: 16
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/utils:config_manager.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 402
- **文件**: /home/liying/sqlcc/src/utils/BUILD.bazel
- **行号**: 17
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/utils:config_snapshot.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 403
- **文件**: /home/liying/sqlcc/src/utils/BUILD.bazel
- **行号**: 18
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/utils:config_lifecycle.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 404
- **文件**: /home/liying/sqlcc/src/utils/BUILD.bazel
- **行号**: 19
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/utils:smart_config_manager.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 405
- **文件**: /home/liying/sqlcc/src/utils/BUILD.bazel
- **行号**: 20
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/utils:ssl_wrapper.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 406
- **文件**: /home/liying/sqlcc/src/sql_parser/BUILD.bazel
- **行号**: 25
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 407
- **文件**: /home/liying/sqlcc/src/sql_parser/BUILD.bazel
- **行号**: 54
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 408
- **文件**: /home/liying/sqlcc/src/sql_parser/BUILD.bazel
- **行号**: 66
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 409
- **文件**: /home/liying/sqlcc/src/sql_parser/BUILD.bazel
- **行号**: 79
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 410
- **文件**: /home/liying/sqlcc/src/sql_parser/BUILD.bazel
- **行号**: 117
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `"//include",`
- **可自动修复**: 否

### 问题 411
- **文件**: /home/liying/sqlcc/src/sql_parser/BUILD.bazel
- **行号**: 22
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: function_ast.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 412
- **文件**: /home/liying/sqlcc/src/sql_parser/BUILD.bazel
- **行号**: 19
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: window_function.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 413
- **文件**: /home/liying/sqlcc/src/types/BUILD.bazel
- **行号**: 47
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 414
- **文件**: /home/liying/sqlcc/src/types/BUILD.bazel
- **行号**: 14
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/types:domain_manager
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 415
- **文件**: /home/liying/sqlcc/src/types/BUILD.bazel
- **行号**: 44
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: domain_manager_test.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 416
- **文件**: /home/liying/sqlcc/src/types/BUILD.bazel
- **行号**: 45
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: types_test.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 417
- **文件**: /home/liying/sqlcc/src/transaction_manager/BUILD.bazel
- **行号**: 9
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 418
- **文件**: /home/liying/sqlcc/src/transaction_manager/BUILD.bazel
- **行号**: 8
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include:transaction_manager_headers
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 419
- **文件**: /home/liying/sqlcc/src/procedure/BUILD.bazel
- **行号**: 11
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/procedure:procedure_parser.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `hdrs = ["//include/procedure:procedure_parser.h"],`
- **可自动修复**: 否

### 问题 420
- **文件**: /home/liying/sqlcc/src/procedure/BUILD.bazel
- **行号**: 12
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 421
- **文件**: /home/liying/sqlcc/src/procedure/BUILD.bazel
- **行号**: 23
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/procedure:procedure_vm.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `hdrs = ["//include/procedure:procedure_vm.h"],`
- **可自动修复**: 否

### 问题 422
- **文件**: /home/liying/sqlcc/src/procedure/BUILD.bazel
- **行号**: 24
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 423
- **文件**: /home/liying/sqlcc/src/procedure/BUILD.bazel
- **行号**: 36
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/procedure:procedure_trigger_executor.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `hdrs = ["//include/procedure:procedure_trigger_executor.h"],`
- **可自动修复**: 否

### 问题 424
- **文件**: /home/liying/sqlcc/src/procedure/BUILD.bazel
- **行号**: 37
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 425
- **文件**: /home/liying/sqlcc/src/procedure/BUILD.bazel
- **行号**: 11
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/procedure:procedure_parser.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 426
- **文件**: /home/liying/sqlcc/src/procedure/BUILD.bazel
- **行号**: 23
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/procedure:procedure_vm.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 427
- **文件**: /home/liying/sqlcc/src/procedure/BUILD.bazel
- **行号**: 36
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/procedure:procedure_trigger_executor.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 428
- **文件**: /home/liying/sqlcc/src/procedure/BUILD.bazel
- **行号**: 60
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: procedure_parser_test.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 429
- **文件**: /home/liying/sqlcc/src/procedure/BUILD.bazel
- **行号**: 70
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: procedure_vm_test.cpp
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 430
- **文件**: /home/liying/sqlcc/src/execution/BUILD.bazel
- **行号**: 19
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include/execution:function_executor.h"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `hdrs = ["//include/execution:function_executor.h"],`
- **可自动修复**: 否

### 问题 431
- **文件**: /home/liying/sqlcc/src/execution/BUILD.bazel
- **行号**: 26
- **类型**: MISSING_TARGET
- **严重程度**: HIGH
- **问题描述**: 依赖目标不存在: //src/trigger:trigger
- **修复建议**: 检查目标名称或创建缺失的目标
- **可自动修复**: 否

### 问题 432
- **文件**: /home/liying/sqlcc/src/execution/BUILD.bazel
- **行号**: 19
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include/execution:function_executor.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 433
- **文件**: /home/liying/sqlcc/src/trigger/BUILD.bazel
- **行号**: 8
- **类型**: MISSING_TARGET
- **严重程度**: HIGH
- **问题描述**: 依赖目标不存在: //src/trigger:trigger
- **修复建议**: 检查目标名称或创建缺失的目标
- **可自动修复**: 否

### 问题 434
- **文件**: /home/liying/sqlcc/src/trigger/BUILD.bazel
- **行号**: 4
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: sql_trigger_executor.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 435
- **文件**: /home/liying/sqlcc/src/trigger/BUILD.bazel
- **行号**: 16
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: trigger_manager.h
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

### 问题 436
- **文件**: /home/liying/sqlcc/src/sql_executor/BUILD.bazel
- **行号**: 11
- **类型**: LABEL_ERROR
- **严重程度**: MEDIUM
- **问题描述**: 标签格式不规范: "//include"
- **修复建议**: 使用标准格式: //package:target
- **上下文**: `includes = ["//include"],`
- **可自动修复**: 否

### 问题 437
- **文件**: /home/liying/sqlcc/src/sql_executor/BUILD.bazel
- **行号**: 10
- **类型**: MISSING_FILE
- **严重程度**: HIGH
- **问题描述**: 引用的文件不存在: //include:sql_executor
- **修复建议**: 检查文件路径或创建缺失的文件
- **可自动修复**: 否

## 依赖关系分析

### ast_nodes_test
- //src/execution:execution
- //src/sql_parser:sql_parser
- @com_google_googletest//:gtest_main

### b_plus_tree_node
- //include/utils:headers
- //include:storage_engine

### b_plus_tree_test
- //src/config_manager:sqlcc_config_manager
- //src/core:core
- //src/execution:execution
- //src/sql_parser:sql_parser
- //src/storage_engine:storage_engine
- //src/utils:utils
- @com_google_googletest//:gtest_main

### buffer_pool_components
- //include/exception:exception
- //include/utils:utils_lib

### buffer_pool_src
- //include/exception:exception
- //include/utils:utils_lib

### buffer_pool_test
- //src/config_manager:sqlcc_config_manager
- //src/storage_engine:storage_engine
- //src/utils:utils
- @com_google_googletest//:gtest_main

### constraint_system_test
- //src/core:core
- //src/execution:execution
- //src/sql_parser:sql_parser
- @com_google_googletest//:gtest_main

### constraint_validation_test
- //src/core:sqlcc_core_lib
- //src/sql_executor:sqlcc_executor
- //src:execution_engine
- //src:unified_executor
- @com_google_googletest//:gtest_main

### core
- //include/exception:exception
- //include:error_handler
- //include:sqlcc_pch

### core_headers
- //include/core:headers

### debug_lexer
- //src/sql_parser:sql_parser

### encryption_key
- :aes_encryptor
- :encryption_key
- :hmac_sha256
- :pbkdf2
- :simple_encryptor

### execution_context_test
- //src/core:core
- //src/core:sqlcc_core_lib
- //src/execution:execution
- //src/sql_parser:sql_parser
- //src:execution_context
- @com_google_googletest//:gtest_main

### expression_test
- //src/sql_parser:sql_parser

### function_executor
- //include/execution:task_executor_hdr
- //include/procedure:procedure_headers
- //include/sql_parser:function_ast
- //src/core:core
- //src/execution:execution
- //src/procedure:procedure
- //src/sql_parser:sql_parser
- //src/trigger:trigger

### grouping_having_clause_test
- //src/core:sqlcc_core_lib
- //src/core:sqlcc_database_core
- //src/execution:execution
- //src/network:network
- //src/procedure:procedure
- //src/sql_executor:sqlcc_executor
- //src/sql_parser:sql_parser
- //src/storage_engine:storage_engine
- //src/trigger:trigger
- //src/types:types
- //src:unified_executor
- @com_google_googletest//:gtest_main

### gtest_sql_executor_tests
- //src/sql_executor:sqlcc_executor
- //src/sql_parser:sql_parser
- @com_google_googletest//:gtest_main

### isql_network_lib
- //src/core:core
- //src/logger:logger
- //src/network:network
- //src/sql_executor:sql_executor

### lazy_writer_test
- //src/storage_engine:lazy_writer
- //src/storage_engine:storage_engine
- //src/utils:utils
- @com_google_googletest//:gtest_main

### lexer_new_unit_test
- //src/core:core
- //src/sql_parser:sql_parser
- @com_google_googletest//:gtest_main

### logger
- //src/core:core
- //src/execution:execution
- //src/sql_parser:sql_parser
- //src/storage_engine:storage_engine

### multi_threaded_network_manager_test
- //include/network:multi_threaded_network_manager
- //src/core:core
- //src/execution:execution
- //src/network:network
- //src/utils:utils
- @com_google_googletest//:gtest_main

### network_unit_test
- //src/network:network
- //src/network:sqlcc_network
- @com_google_googletest//:gtest_main

### page_raii
- //include/utils:headers
- //include:storage_engine

### parser_create_table_test
- //src/core:core
- //src/execution:execution
- //src/procedure:procedure
- //src/sql_parser:sql_parser
- //src/storage_engine:storage_engine
- //src/transaction:transaction
- //src/trigger:trigger
- //src/types:types
- //src/utils:utils
- @com_google_googletest//:gtest_main

### performance_test_base
- //src/core:sqlcc_core_lib
- //src/sql_executor:sqlcc_executor
- //src:sqlcc_core

### procedure_parser
- //src/procedure:procedure
- //src/sql_parser:sql_parser

### server_main
- //src/core:core
- //src/network:network
- //src/network:sqlcc_network
- //src/sql_executor:sqlcc_executor
- //src/sql_parser:sql_parser
- //src/storage_engine:index_manager
- //src/storage_engine:storage_engine
- //src/transaction_manager:sqlcc_transaction_manager
- //src:unified_executor

### session
- //include/network/encryption:aes_encryptor

### simple_constraint_test
- //src/core:core
- //src/procedure:procedure
- //src/sql_parser:sql_parser
- //src/storage_engine:storage_engine
- @com_google_googletest//:gtest_main

### sql_executor
- //src/core:core
- //src/execution:execution
- //src/logger:logger
- //src/sql_parser:sql_parser
- //src/storage_engine:storage_engine
- //src/utils:utils

### sql_parser
- //include/sql_parser:ast_node
- //include/sql_parser:ast_nodes
- //include/sql_parser:constraint
- //include/sql_parser:node_visitor
- //include/sql_parser:token
- //include/storage:storage_headers_lib
- //src/types:types

### sql_parser_high_coverage_test
- //src/sql_parser:sql_parser
- @com_google_googletest//:gtest_main

### sql_trigger_executor
- //include/trigger:trigger_headers
- //src/core:core
- //src/trigger:trigger

### sqlcc
- //src/core:core
- //src/logger:logger
- //src/network:network
- //src/sql_executor:sql_executor
- //src/sql_parser:sql_parser
- //src/storage_engine:storage_engine
- //src/utils:utils

### sqlcc_config_manager
- //src/utils:utils
- @com_google_absl//absl/strings

### storage_engine
- //include/sql_parser:headers
- //include/storage_engine/table_storage:page_raii
- //include/storage_engine/table_storage:record_validator
- //include/utils:headers
- //include:disk_manager
- //include:exception
- //include:page
- //include:storage_engine
- //src/sql_parser:sql_parser
- //src/utils:utils

### stored_procedure_manager
- //include:database_manager
- //include:execution_engine

### stored_procedure_manager_test
- //include/core:stored_procedure_manager
- //src/core:core
- //src/utils:utils
- @com_google_googletest//:gtest_main

### task_executor_test
- //src/core:core
- //src/execution:execution
- //src/sql_parser:sql_parser
- //src/storage_engine:storage_engine
- @com_google_googletest//:gtest_main

### test_task_executor
- //src/sql_parser:sql_parser

### transaction
- //include/storage:headers
- //include/utils:headers
- //src/transaction:transaction

### transaction_manager_test
- //src/transaction:transaction
- //src/transaction_manager:sqlcc_transaction_manager
- @com_google_googletest//:gtest_main

### types
- //include/core:headers
- //include/procedure:procedure_headers
- //include/types:domain_manager
- //include/utils:headers
