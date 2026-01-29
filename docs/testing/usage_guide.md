# SQLCC 测试框架使用指南

## 快速开始

### 环境要求

**系统要求**:
- Linux/Unix 系统
- CMake 3.10+
- GCC 7.0+ 或 Clang 5.0+
- Python 3.6+ (用于性能测试)

**可选依赖**:
- lcov (覆盖率报告)
- gcov (覆盖率分析)
- graphviz (性能图表)

### 基础测试执行

#### 运行所有测试
```bash
# 进入项目根目录
cd /home/liying/sqlcc

# 运行完整测试套件
./scripts/run_tests.sh

# 或者使用简化的测试脚本
./scripts/run_all_tests.sh
```

#### 运行特定类型测试
```bash
# 仅运行单元测试
./scripts/run_tests.sh --unit

# 仅运行集成测试
./scripts/run_tests.sh --integration

# 仅运行性能测试
./scripts/run_tests.sh --performance

# 运行覆盖率测试
./scripts/run_tests.sh --coverage
```

### 测试结果解读

#### 测试状态说明
- ✅ **PASS**: 测试通过
- ❌ **FAIL**: 测试失败
- ⚠️ **SKIP**: 测试跳过
- 🔄 **RUNNING**: 测试运行中

#### 性能指标解读
- **延迟**: 单次操作耗时 (目标: <5ms)
- **吞吐量**: 单位时间处理量
- **资源使用**: CPU、内存、磁盘I/O

## 详细使用说明

### 主测试脚本 (run_tests.sh)

#### 功能概述
`run_tests.sh` 是SQLCC项目的核心测试脚本，提供完整的测试执行、覆盖率分析和报告生成功能。

#### 命令行参数
```bash
# 基本用法
./scripts/run_tests.sh [选项]

# 常用选项
--coverage           # 启用覆盖率测试
--parallel=N         # 并行测试 (默认: 禁用)
--unit               # 仅运行单元测试
--integration        # 仅运行集成测试
--performance        # 仅运行性能测试
--quick              # 快速测试模式
--verbose            # 详细输出模式
--help               # 显示帮助信息
```

#### 使用示例
```bash
# 完整测试套件
./scripts/run_tests.sh --coverage --parallel=4

# 快速验证
./scripts/run_tests.sh --quick

# 详细调试
./scripts/run_tests.sh --verbose --unit
```

### 覆盖率测试 (generate_coverage_report.sh)

#### 功能概述
生成详细的代码覆盖率报告，包括HTML格式的可视化报告。

#### 使用说明
```bash
# 生成覆盖率报告
./scripts/generate_coverage_report.sh

# 指定输出目录
./scripts/generate_coverage_report.sh --output=coverage_reports/

# 清理旧数据并重新生成
./scripts/generate_coverage_report.sh --clean
```

#### 报告解读
- **行覆盖率**: 代码行执行比例
- **分支覆盖率**: 条件分支覆盖比例
- **函数覆盖率**: 函数调用覆盖比例

### 性能测试脚本

#### CRUD性能测试 (run_crud_performance.sh)
```bash
# 运行CRUD性能测试
./scripts/shell/run_crud_performance.sh

# 指定数据规模
./scripts/shell/run_crud_performance.sh --rows=100000

# 指定测试类型
./scripts/shell/run_crud_performance.sh --test-type=insert
```

#### 并发性能测试 (run_concurrent_performance.sh)
```bash
# 运行并发性能测试
./scripts/shell/run_concurrent_performance.sh

# 指定并发数
./scripts/shell/run_concurrent_performance.sh --threads=10
```

#### CPU性能测试 (run_cpu_performance.sh)
```bash
# 运行CPU性能测试
./scripts/shell/run_cpu_performance.sh

# 指定测试时长
./scripts/shell/run_cpu_performance.sh --duration=60
```

### 特定功能测试

#### SQL执行器测试 (run_sql_executor_tests.sh)
```bash
# 运行SQL执行器测试
./scripts/shell/run_sql_executor_tests.sh

# 测试特定功能
./scripts/shell/run_sql_executor_tests.sh --test-ast
./scripts/shell/run_sql_executor_tests.sh --test-execution
./scripts/shell/run_sql_executor_tests.sh --test-integration
```

#### 解析器测试 (run_parser_refactor_tests.sh)
```bash
# 运行解析器测试
./scripts/shell/run_parser_refactor_tests.sh

# 测试特定组件
./scripts/shell/run_parser_refactor_tests.sh --test-ast-core
./scripts/shell/run_parser_refactor_tests.sh --test-expression
./scripts/shell/run_parser_refactor_tests.sh --test-integration
```

## 高级用法

### 自定义测试配置

#### 环境变量配置
```bash
# 设置测试超时时间
export TEST_TIMEOUT=300

# 设置构建目录
export BUILD_DIR=build_test_custom

# 启用详细日志
export VERBOSE=1

# 运行测试
./scripts/run_tests.sh
```

#### 配置文件方式
创建 `test_config.cfg` 文件:
```ini
[general]
timeout=300
build_dir=build_test_custom
verbose=true

[coverage]
enable=true
output_dir=coverage_reports/

[performance]
threads=4
duration=60
```

使用配置文件:
```bash
./scripts/run_tests.sh --config=test_config.cfg
```

### 并行测试优化

#### 并行测试配置
```bash
# 根据CPU核心数自动设置并行度
NUM_CORES=$(nproc)
./scripts/run_tests.sh --parallel=$NUM_CORES

# 限制并行度以避免资源竞争
./scripts/run_tests.sh --parallel=2
```

#### 性能优化建议
- 使用SSD存储提高I/O性能
- 确保足够的内存可用
- 避免在负载高的系统上运行测试
- 定期清理测试缓存

### 测试结果分析

#### 性能基准测试
```bash
# 运行基准测试
./scripts/shell/run_crud_performance.sh --baseline

# 比较当前性能与基准
./scripts/shell/run_crud_performance.sh --compare-baseline
```

#### 回归测试
```bash
# 运行回归测试套件
./scripts/run_tests.sh --regression

# 指定回归测试版本
./scripts/run_tests.sh --regression --version=v1.0.8
```

## 故障排除

### 常见问题

#### 构建失败
**问题**: 测试构建失败
**解决方案**:
```bash
# 清理构建目录
rm -rf build_test*

# 重新构建
./scripts/run_tests.sh --clean
```

#### 测试超时
**问题**: 测试执行超时
**解决方案**:
```bash
# 增加超时时间
export TEST_TIMEOUT=600
./scripts/run_tests.sh

# 或者使用快速模式
./scripts/run_tests.sh --quick
```

#### 覆盖率报告生成失败
**问题**: 覆盖率报告无法生成
**解决方案**:
```bash
# 检查依赖
which lcov
which genhtml

# 重新安装依赖
sudo apt-get install lcov

# 重新生成报告
./scripts/generate_coverage_report.sh --clean
```

### 调试技巧

#### 详细日志输出
```bash
# 启用详细日志
./scripts/run_tests.sh --verbose

# 保存日志到文件
./scripts/run_tests.sh --verbose 2>&1 | tee test_log.txt
```

#### 单步调试
```bash
# 运行单个测试文件
cd tests/sql_parser/
./test_lexer_parser

# 使用gdb调试
gdb ./test_lexer_parser
```

#### 性能分析
```bash
# 使用perf进行性能分析
perf record ./scripts/shell/run_crud_performance.sh
perf report

# 使用valgrind检查内存
valgrind --tool=memcheck ./scripts/shell/run_sql_executor_tests.sh
```

## 最佳实践

### 测试执行最佳实践

#### 常规测试流程
1. **环境准备**: 确保所有依赖已安装
2. **构建验证**: 运行快速构建测试
3. **功能测试**: 执行核心功能测试
4. **性能测试**: 验证性能指标
5. **覆盖率测试**: 检查代码覆盖率
6. **报告生成**: 生成测试报告

#### 持续集成集成
```yaml
# GitHub Actions 示例
name: SQLCC Tests
on: [push, pull_request]

jobs:
  test:
    runs-on: ubuntu-latest
    steps:
    - uses: actions/checkout@v2
    - name: Run Tests
      run: |
        chmod +x scripts/run_tests.sh
        ./scripts/run_tests.sh --coverage --parallel=2
    - name: Upload Coverage
      uses: codecov/codecov-action@v2
```

### 性能测试最佳实践

#### 测试环境一致性
- 使用相同的硬件配置
- 确保系统负载稳定
- 避免后台进程干扰
- 多次运行取平均值

#### 数据规模选择
- 小规模: 功能验证 (1-1,000行)
- 中等规模: 性能测试 (1,000-100,000行)
- 大规模: 压力测试 (100,000+行)

### 覆盖率测试最佳实践

#### 覆盖率目标
- **核心功能**: 90%+ 行覆盖率
- **关键模块**: 95%+ 行覆盖率
- **边界条件**: 充分测试异常情况

#### 覆盖率优化
- 添加缺失的测试用例
- 测试异常和边界条件
- 定期审查覆盖率报告
- 设置覆盖率阈值

## 扩展和定制

### 添加新的测试脚本

#### 测试脚本模板
```bash
#!/bin/bash

# 脚本信息
SCRIPT_NAME="$(basename "$0")"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# 导入公共函数
source "${SCRIPT_DIR}/../utils/common.sh"

# 参数解析
parse_args() {
    while [[ $# -gt 0 ]]; do
        case $1 in
            --help|-h)
                show_help
                exit 0
                ;;
            --verbose|-v)
                VERBOSE=1
                shift
                ;;
            *)
                echo "未知参数: $1"
                exit 1
                ;;
        esac
    done
}

# 主函数
main() {
    parse_args "$@"
    
    # 环境检测
    check_environment
    
    # 测试执行
    run_tests
    
    # 结果报告
    generate_report
}

# 执行主函数
main "$@"
```

### 自定义测试套件

#### 创建自定义套件
```cmake
# 在 tests/CMakeLists.txt 中添加
add_custom_target(custom_suite
    COMMAND ctest -R "^test_.*" --output-on-failure
    COMMENT "运行自定义测试套件"
    DEPENDS sqlcc_test_framework
)
```

#### 集成到主测试框架
```bash
# 在 run_tests.sh 中添加
run_custom_tests() {
    echo "运行自定义测试套件"
    cd "${BUILD_DIR}"
    make custom_suite
    
    if [ $? -eq 0 ]; then
        echo "✅ 自定义测试通过"
    else
        echo "❌ 自定义测试失败"
        return 1
    fi
}
```

## 附录

### 命令行参数参考

#### run_tests.sh 完整参数
```bash
--help, -h             显示帮助信息
--coverage, -c         启用覆盖率测试
--parallel=N, -p N     并行测试数
--unit, -u             仅运行单元测试
--integration, -i      仅运行集成测试
--performance, -P      仅运行性能测试
--quick, -q            快速测试模式
--verbose, -v          详细输出模式
--clean, -C            清理构建目录
--config=FILE          使用配置文件
--output=DIR           指定输出目录
--timeout=SEC          设置超时时间
```

### 环境变量参考

#### 测试相关环境变量
```bash
TEST_TIMEOUT=300        # 测试超时时间(秒)
BUILD_DIR=build_test    # 构建目录
VERBOSE=1              # 详细输出
COVERAGE=1             # 启用覆盖率
PARALLEL=4             # 并行测试数
```

### 文件位置参考

#### 重要文件位置
```
项目根目录/
├── scripts/           # 测试脚本目录
│   ├── run_tests.sh              # 主测试脚本
│   ├── generate_coverage_report.sh # 覆盖率脚本
│   └── shell/                    # Shell测试脚本
├── tests/             # 测试代码目录
│   ├── unit/          # 单元测试
│   ├── integration/   # 集成测试
│   └── performance/   # 性能测试
└── docs/tests/        # 测试文档
    ├── README.md          # 框架概述
    ├── design_and_improvements.md # 设计文档
    └── usage_guide.md     # 使用指南
```

---

*本文档最后更新: 2025年12月*  
*维护者: SQLCC开发团队*  
*版本: v1.1.1*