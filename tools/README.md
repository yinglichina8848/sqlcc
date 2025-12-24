# SQLCC 开发工具集

## 概述

SQLCC开发工具集提供了一系列自动化工具，帮助开发者提高开发效率、维护代码质量和自动化重复性任务。

## 工具列表

### 1. Bazel标签自动修复器 (`bazel_label_auto_fixer.py`)

#### 功能特性
- **自动扫描**: 扫描项目中所有BUILD.bazel文件
- **智能检测**: 检测Bazel标签格式错误，包括：
  - 无效的外部引用格式 (`//package:subpackage/file.cpp`)
  - 缺少引号的标签引用
  - 相对路径错误
- **安全修复**: 支持dry-run模式预览修复内容
- **批量处理**: 支持单文件和全项目批量修复
- **详细报告**: 生成修复报告和统计信息

#### 使用方法

```bash
# 预览模式（推荐先运行）
python3 tools/bazel_label_auto_fixer.py --dry-run --verbose

# 实际修复
python3 tools/bazel_label_auto_fixer.py --fix

# 修复特定文件
python3 tools/bazel_label_auto_fixer.py --fix --filter "src/BUILD.bazel"

# 生成详细报告
python3 tools/bazel_label_auto_fixer.py --dry-run --report bazel_fix_report.md
```

#### 修复示例

**修复前:**
```bazel
deps = [
    "//src:network/network.cpp",  # 错误格式
    //src:subpackage/file.cpp,     # 缺少引号
    "../../include/utils.h",      # 相对路径
]
```

**修复后:**
```bazel
deps = [
    "//src/network:network.cpp",  # 正确格式
    "//src/subpackage:file.cpp",   # 添加引号
    "//include:utils.h",          # 简化路径
]
```

#### 测试结果

在SQLCC项目中测试，发现并修复了49个Bazel标签问题：
- 处理文件总数: 71个
- 发现问题总数: 49个
- 无效的外部引用格式: 4个
- 缺少引号: 33个
- 相对路径错误: 12个

### 2. 依赖分析器 (`test_dependency_analyzer.py`)

#### 功能特性
- 分析测试文件依赖关系
- 生成依赖图
- 识别循环依赖
- 优化依赖层次

### 3. 测试配置验证器 (`test_config_validator.py`)

#### 功能特性
- 验证测试配置文件
- 检查测试环境设置
- 生成配置报告

### 4. 头文件使用分析器 (`analyze_header_usage.py`)

#### 功能特性
- 分析头文件使用情况
- 生成包含关系图
- 识别冗余包含

## 安装和配置

### 环境要求
- Python 3.7+
- 必要的Python包会自动安装

### 快速开始
```bash
# 克隆项目
git clone https://gitee.com/yinglichina/sqlcc.git
cd sqlcc

# 运行Bazel标签修复器
python3 tools/bazel_label_auto_fixer.py --dry-run
```

## 使用建议

### 1. 开发前检查
```bash
# 检查BUILD文件是否有格式问题
python3 tools/bazel_label_auto_fixer.py --dry-run
```

### 2. 持续集成
将工具集成到CI/CD流程中：
```yaml
# .github/workflows/ci.yml
- name: Check Bazel Labels
  run: python3 tools/bazel_label_auto_fixer.py --dry-run
```

### 3. 定期维护
```bash
# 每月运行一次全面检查
python3 tools/bazel_label_auto_fixer.py --dry-run --report monthly_report.md
```

## 贡献指南

### 添加新工具
1. 在`tools/`目录下创建Python脚本
2. 添加详细的文档字符串
3. 提供命令行接口
4. 更新本README文件
5. 添加使用示例

### 代码规范
- 使用类型注解
- 提供详细的错误处理
- 支持命令行参数
- 生成有用的输出信息

## 许可证

本工具集遵循与SQLCC项目相同的许可证。

## 联系方式

如有问题或建议，请通过以下方式联系：
- 提交Issue: [SQLCC Issues](https://gitee.com/yinglichina/sqlcc/issues)
- 讨论区: [SQLCC Discussions](https://gitee.com/yinglichina/sqlcc/discussions)

---

**SQLCC开发工具集** - 让开发更高效，让代码更高质量
