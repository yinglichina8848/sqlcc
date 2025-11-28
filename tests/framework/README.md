# SQLCC 测试框架

这是SQLCC项目的统一测试框架，提供了完整的单元测试、集成测试、性能测试和覆盖率测试功能。

## 目录结构

```
framework/
├── CMakeLists.txt          # 框架CMake配置
├── test_base.h             # 测试基类定义
├── gtest/                  # Google Test集成
│   └── gtest_main.cpp      # 测试主入口
├── reporting/              # 测试报告生成
│   ├── test_reporter.h     # 报告生成器头文件
│   └── test_reporter.cpp   # 报告生成器实现
├── common/                 # 公共测试工具
│   ├── mock_objects.h      # Mock对象定义
│   ├── test_utils.h        # 测试工具函数头文件
│   └── test_utils.cpp      # 测试工具函数实现
└── README.md               # 框架说明文档
```

## 功能特点

1. **统一测试框架**: 基于Google Test构建，提供一致的测试接口
2. **多层次测试**: 支持单元测试、集成测试、性能测试
3. **覆盖率测试**: 集成gcov/lcov，支持代码覆盖率分析
4. **自动化报告**: 自动生成HTML格式测试报告
5. **Mock支持**: 提供常用组件的Mock对象
6. **并行执行**: 支持测试并行执行，提高测试效率

## 使用方法

### 构建测试

```bash
cd build
cmake ..
make
```

### 运行测试

```bash
# 运行所有测试
ctest

# 运行特定测试
ctest -R "UnitTests"

# 详细输出
ctest --verbose
```

### 生成覆盖率报告

```bash
# 启用覆盖率
cmake -DENABLE_COVERAGE=ON ..
make
ctest
make unit_tests_coverage_report
```

## 测试类型

### 单元测试 (unit/)

针对单个函数或类进行测试，验证其功能正确性。

### 集成测试 (integration/)

测试多个模块之间的交互，验证整体功能。

### 性能测试 (performance/)

测试系统性能，包括基准测试和压力测试。

## 扩展性

可以通过以下方式扩展测试框架：

1. 添加新的测试基类到[test_base.h](test_base.h)
2. 添加Mock对象到[mock_objects.h](mock_objects.h)
3. 添加工具函数到[test_utils.h](test_utils.h)和[test_utils.cpp](test_utils.cpp)
4. 添加新的报告模板到[reporting/](reporting/)目录

## 依赖项

- Google Test 1.10+
- Google Benchmark (性能测试)
- gcov/lcov (覆盖率测试)
- CMake 3.16+

## 配置文件

- [../config/test_config.yaml](../config/test_config.yaml): 测试配置文件
- [../config/coverage_config.cmake](../config/coverage_config.cmake): 覆盖率配置

## 相关文档

- [../README.md](../README.md): 项目主文档
- [../../docs/testing.md](../../docs/testing.md): 详细测试文档