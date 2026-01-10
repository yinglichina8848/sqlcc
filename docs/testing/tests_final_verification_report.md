# Bazel标签自动修复报告
生成时间: report

## 统计摘要
- 处理文件总数: 71
- 实际处理文件: 25
- 发现问题总数: 8

## 问题类型统计
- 相对路径错误: 8 个

## 详细修复记录
### /home/liying/sqlcc/tests/components/core/BUILD.bazel (7 个修复)
#### 修复 1
- **行号**: 9
- **错误类型**: 相对路径错误
- **原始代码**: `#     includes = ["../include", "../../../include/core", "../../../include/storage", "../../../src/sql_executor"],`
- **修复后**: `#     includes = ["../include", "../../../include/core", "../../../include/storage", "../../../src/sql_executor"],`
- **描述**: 简化相对路径: ../../ -> 

#### 修复 2
- **行号**: 9
- **错误类型**: 相对路径错误
- **原始代码**: `#     includes = ["../include", "../../../include/core", "../../../include/storage", "../../../src/sql_executor"],`
- **修复后**: `#     includes = ["../include", "../../../include/core", "../../../include/storage", "../../../src/sql_executor"],`
- **描述**: 简化相对路径: ../../ -> 

#### 修复 3
- **行号**: 9
- **错误类型**: 相对路径错误
- **原始代码**: `#     includes = ["../include", "../../../include/core", "../../../include/storage", "../../../src/sql_executor"],`
- **修复后**: `#     includes = ["../include", "../../../include/core", "../../../include/storage", "../../../src/sql_executor"],`
- **描述**: 简化相对路径: ../../ -> 

#### 修复 4
- **行号**: 23
- **错误类型**: 相对路径错误
- **原始代码**: `includes = ["../include", "../../../include/core"],`
- **修复后**: `includes = ["../include", "../../../include/core"],`
- **描述**: 简化相对路径: ../../ -> 

#### 修复 5
- **行号**: 45
- **错误类型**: 相对路径错误
- **原始代码**: `includes = ["../include", "../../../include/core"],`
- **修复后**: `includes = ["../include", "../../../include/core"],`
- **描述**: 简化相对路径: ../../ -> 

#### 修复 6
- **行号**: 67
- **错误类型**: 相对路径错误
- **原始代码**: `includes = ["../include", "../../../include/core"],`
- **修复后**: `includes = ["../include", "../../../include/core"],`
- **描述**: 简化相对路径: ../../ -> 

#### 修复 7
- **行号**: 88
- **错误类型**: 相对路径错误
- **原始代码**: `includes = ["../include", "../../../include/core"],`
- **修复后**: `includes = ["../include", "../../../include/core"],`
- **描述**: 简化相对路径: ../../ -> 


### /home/liying/sqlcc/tests/unit/basic/BUILD.bazel (1 个修复)
#### 修复 1
- **行号**: 54
- **错误类型**: 相对路径错误
- **原始代码**: `includes = ["../../../include"],`
- **修复后**: `includes = ["../../../include"],`
- **描述**: 简化相对路径: ../../ -> 

