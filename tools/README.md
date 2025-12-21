# SQLCC 工具集

本目录包含SQLCC项目使用的各种辅助工具。

## 工具列表

### bazel_label_fixer_enhanced.py
增强版Bazel标签修复工具，用于自动修复BUILD文件中的标签路径问题。

#### 功能特性
- 修复 `//include:xxx.h` 标签为 `//include/xxx:xxx.h` 格式
- 修复 `//src:xxx.cpp` 标签为 `//src/xxx:xxx.cpp` 格式
- 自动转换相对路径引用为绝对路径：
  - `../../include` → `//include`
  - `../../../include` → `//include`
- 支持单个文件或整个目录的批量修复
- 提供dry-run模式预览修复效果

#### 使用方法
```bash
# 给工具添加执行权限
chmod +x bazel_label_fixer_enhanced.py

# 查看帮助信息
python3 bazel_label_fixer_enhanced.py --help

# dry-run模式：预览需要修复的文件（不实际修改）
python3 bazel_label_fixer_enhanced.py /path/to/build/file.or.directory --dry-run

# 实际修复文件
python3 bazel_label_fixer_enhanced.py /path/to/build/file.or.directory

# 修复整个项目
python3 bazel_label_fixer_enhanced.py .
```

#### 使用示例
```bash
# 修复当前目录下的所有BUILD文件（预览模式）
python3 bazel_label_fixer_enhanced.py . --dry-run

# 修复特定BUILD文件
python3 bazel_label_fixer_enhanced.py src/core/BUILD.bazel

# 修复整个src目录
python3 bazel_label_fixer_enhanced.py src
```

### bazel_dep_fixer_enhanced.py
增强版BUILD文件依赖修复工具，用于自动修复BUILD文件中的依赖声明问题。

#### 功能特性
- 自动修复缺失的依赖声明
- 移除未使用的依赖项
- 优化依赖声明顺序
- 修复相对路径依赖问题
- 支持单个文件或整个目录的批量修复
- 提供dry-run模式预览修复效果

#### 使用方法
```bash
# 给工具添加执行权限
chmod +x bazel_dep_fixer_enhanced.py

# 查看帮助信息
python3 bazel_dep_fixer_enhanced.py --help

# dry-run模式：预览需要修复的文件（不实际修改）
python3 bazel_dep_fixer_enhanced.py /path/to/build/file.or.directory --dry-run

# 实际修复文件
python3 bazel_dep_fixer_enhanced.py /path/to/build/file.or.directory

# 修复整个项目
python3 bazel_dep_fixer_enhanced.py .
```

#### 使用示例
```bash
# 修复当前目录下的所有BUILD文件（预览模式）
python3 bazel_dep_fixer_enhanced.py . --dry-run

# 修复特定BUILD文件
python3 bazel_dep_fixer_enhanced.py src/core/BUILD.bazel

# 修复整个src目录
python3 bazel_dep_fixer_enhanced.py src
```

### migrate_config_manager.sh
配置管理器迁移工具，用于将旧版配置迁移到新版配置系统。

### build_validator.sh
构建验证工具，用于验证项目构建配置的正确性。

### bazel_check_deps.sh
Bazel依赖检查工具，用于分析和报告Bazel构建依赖关系。

### bazel_code_checker.py
Bazel代码检查工具，用于检查代码质量和规范。

## 目录结构

- `cpp/` - C++相关工具
- `mcp/` - MCP协议相关工具
- `test_framework/` - 测试框架相关工具

## 使用注意事项

1. 所有工具都应该在项目根目录下运行
2. 在运行任何修改性工具之前，建议先使用dry-run模式预览效果
3. 定期更新工具以获得最新功能和修复