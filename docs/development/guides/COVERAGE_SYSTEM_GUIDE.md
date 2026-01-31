# SQLCC 统一覆盖率测试系统使用指南

## 概述

SQLCC 统一覆盖率测试系统是一个集成的测试和分析框架，整合了所有覆盖率相关的脚本和工具，提供统一的接口来执行覆盖率测试、数据收集、分析和报告生成。

## 系统架构

### 核心组件

1. **统一测试脚本** (`scripts/run_unified_coverage.sh`)
   - 主入口脚本，整合所有覆盖率功能
   - 支持多种测试模式和配置选项

2. **质量检查脚本** (`scripts/check_coverage_quality.sh`)
   - 覆盖率质量门禁检查
   - 自动生成质量评估报告

3. **Bazel集成** (`BUILD.bazel`)
   - 提供多种预定义的bazel目标
   - 支持配置化构建和测试

### 支持的测试模式

- **basic**: 基础覆盖率测试
- **comprehensive**: Level 1-6 综合覆盖率测试
- **incremental**: 增量覆盖率测试
- **performance**: 性能覆盖率测试
- **security**: 安全覆盖率测试

### 支持的输出格式

- **html**: HTML 可视化报告
- **lcov**: LCOV 格式报告
- **json**: JSON 结构化数据
- **text**: 文本格式报告
- **all**: 生成所有格式

### 支持的分析深度

- **file**: 文件级分析
- **function**: 函数级分析
- **module**: 模块级分析
- **component**: 组件级分析
- **system**: 系统级分析

## 快速开始

### 1. 基础覆盖率测试

```bash
# 使用脚本直接运行
./scripts/run_unified_coverage.sh --mode basic --format html

# 或者使用bazel目标
bazel test //:coverage_basic
```

### 2. 综合覆盖率测试

```bash
# 运行Level 1-6完整测试
./scripts/run_unified_coverage.sh --mode comprehensive --format all

# 使用bazel
bazel test //:coverage_comprehensive
```

### 3. 自定义配置测试

```bash
# 自定义配置：性能测试 + HTML报告 + 模块级分析
./scripts/run_unified_coverage.sh \
  --mode performance \
  --format html \
  --depth module \
  --threshold 75
```

## Bazel 目标说明

### 主要目标

- `//:coverage_test` - 主要的覆盖率测试目标，支持配置
- `//:coverage_quality_gate` - 质量门禁检查
- `//:coverage_trend_analysis` - 趋势分析
- `//:generate_coverage_reports` - 报告生成
- `//:collect_coverage_data` - 数据收集
- `//:verify_coverage` - 数据验证

### 便捷别名

- `//:test_coverage` - 等同于 coverage_test
- `//:coverage_basic` - 基础模式
- `//:coverage_comprehensive` - 综合模式
- `//:coverage_report` - 生成报告
- `//:coverage_quality_check` - 质量检查

### 测试套件

- `//:coverage_ci` - CI/CD 集成测试套件
- `//:coverage_dev` - 开发人员常用测试套件

## 配置选项

### 命令行参数

```bash
./scripts/run_unified_coverage.sh [选项]

选项:
  -m, --mode MODE       测试模式 (basic|comprehensive|incremental|performance|security)
  -f, --format FORMAT   输出格式 (html|lcov|json|text|all)
  -d, --depth DEPTH     分析深度 (file|function|module|component|system)
  -t, --threshold PCT   质量门禁阈值 (50-95)
  -o, --output DIR      指定输出目录
  -s, --step STEP       执行特定步骤
  -l, --list-steps      列出所有可用步骤
  -v, --verbose         详细输出
  -h, --help            显示帮助信息
```

### Bazel 配置

通过命令行标志配置：

```bash
# 设置测试模式
bazel test //:coverage_test --//:coverage_mode=comprehensive

# 设置输出格式
bazel test //:coverage_test --//:coverage_format=html

# 设置分析深度
bazel test //:coverage_test --//:coverage_depth=module

# 设置质量阈值
bazel test //:coverage_test --//:coverage_threshold=80
```

## 测试执行流程

### 完整流程

1. **环境设置** - 检查依赖工具，创建目录结构
2. **数据收集** - 执行测试，收集覆盖率数据
3. **数据分析** - 分析覆盖率统计，生成洞察
4. **报告生成** - 生成各种格式的报告
5. **质量检查** - 验证是否达到质量标准
6. **清理工作** - 清理临时文件，压缩历史数据

### 特定步骤执行

```bash
# 只执行环境设置
./scripts/run_unified_coverage.sh --step setup

# 只执行数据收集
./scripts/run_unified_coverage.sh --step data_collection --mode comprehensive

# 只生成报告
./scripts/run_unified_coverage.sh --step reporting --format html
```

## 输出结构

```
coverage_results_YYYYMMDD_HHMMSS/
├── coverage_data/           # 原始覆盖率数据
│   ├── *.profdata          # LLVM 覆盖率数据
│   └── *.lcov              # LCOV 格式数据
├── coverage_reports/        # 生成的报告
│   ├── index.html          # HTML 报告首页
│   ├── coverage.lcov       # LCOV 报告
│   ├── coverage.json       # JSON 数据
│   └── coverage.txt        # 文本报告
├── analysis/                # 分析结果
│   ├── module_analysis.json
│   └── trend_data.json
├── logs/                    # 执行日志
└── test_status.json         # 执行状态跟踪
```

## 质量门禁

### 阈值设置

系统支持配置不同的质量阈值：

- **70%**: 标准质量要求
- **75%**: 较高质量要求
- **80%**: 优秀质量要求
- **85%**: 严苛质量要求

### 检查内容

1. **行覆盖率检查** - 整体代码行覆盖率
2. **函数覆盖率检查** - 函数级覆盖率
3. **质量评估** - 生成详细的质量报告

### CI/CD 集成

```yaml
# GitHub Actions 示例
- name: Coverage Quality Check
  run: |
    ./scripts/check_coverage_quality.sh coverage_reports 75
```

## 故障排除

### 常见问题

#### 1. 工具依赖缺失

```bash
# 检查并安装必要工具
./scripts/run_unified_coverage.sh --step setup
```

#### 2. 测试执行失败

```bash
# 查看详细日志
./scripts/run_unified_coverage.sh --verbose --mode basic

# 检查测试状态
cat coverage_results/test_status.json
```

#### 3. 报告生成失败

```bash
# 单独生成报告
./scripts/run_unified_coverage.sh --step reporting --format html

# 检查覆盖率数据
ls -la coverage_results/coverage_data/
```

#### 4. 质量检查失败

```bash
# 查看质量报告
cat coverage_reports/coverage_quality_report.md

# 降低阈值重新检查
./scripts/check_coverage_quality.sh coverage_reports 60
```

### 调试模式

启用详细输出获取更多调试信息：

```bash
./scripts/run_unified_coverage.sh --verbose --mode basic
```

## 最佳实践

### 开发阶段

1. **日常开发**: 使用 `basic` 模式快速检查
2. **功能开发**: 使用 `incremental` 模式验证新代码
3. **代码审查**: 使用 `module` 深度分析组件覆盖

### CI/CD 集成

1. **快速检查**: 在PR检查中使用 `basic` 模式
2. **完整测试**: 在主分支使用 `comprehensive` 模式
3. **质量门禁**: 设置合适的阈值确保代码质量

### 性能优化

1. **并行执行**: 利用bazel的并行测试能力
2. **增量测试**: 只测试变更的代码
3. **缓存利用**: 重用之前的测试结果

## 扩展开发

### 添加新的测试模式

1. 在 `run_unified_coverage.sh` 中添加新的模式处理逻辑
2. 更新 `BUILD.bazel` 中的配置选项
3. 添加对应的测试执行函数

### 添加新的分析功能

1. 创建新的分析脚本
2. 在统一脚本中集成新的分析步骤
3. 更新报告生成逻辑

### 自定义质量检查

1. 修改 `check_coverage_quality.sh`
2. 添加新的检查指标
3. 更新质量报告格式

## 版本历史

- **v1.0.0**: 初始版本，支持基础覆盖率测试
- **v1.1.0**: 添加综合测试模式和多种输出格式
- **v1.2.0**: 集成质量门禁和趋势分析
- **v1.3.0**: 支持增量测试和性能测试模式

## 支持

如遇到问题，请查看：

1. [故障排除](#故障排除) 章节
2. 项目 issue 跟踪
3. 开发团队技术支持

---

**最后更新**: 2026-01-31
**版本**: v1.3.9
