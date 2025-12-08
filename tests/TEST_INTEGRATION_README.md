# SQLCC v1.1.1 集成测试框架

## 概述
已将所有测试整合到统一的测试框架中，分为3大类：
- **Unit测试**: 44个单元测试文件
- **Performance测试**: 25个性能测试文件  
- **Coverage测试**: 1个覆盖率测试文件

## 目录结构
```
tests/
├── CMakeLists.txt                    # 主配置文件（已集成测试框架）
├── CMakeLists_integrated.txt         # 集成测试配置文件
├── unit/                            # 单元测试目录
│   ├── basic/                       # 基础单元测试
│   ├── advanced/                    # 高级单元测试
│   └── core/                        # 核心组件测试
├── performance/                     # 性能测试目录
│   ├── basic/                       # 基础性能测试
│   ├── memory/                      # 内存性能测试
│   ├── concurrency/                 # 并发性能测试
│   ├── stress/                      # 压力测试
│   └── benchmark/                   # 基准测试
└── coverage/                        # 覆盖率测试目录
    ├── unit/                        # 单元测试覆盖率
    └── integration/                 # 集成测试覆盖率
```

## 使用方法
### 1. 编译所有测试
```bash
cmake -B build -DENABLE_INTEGRATED_TEST_FRAMEWORK=ON
cmake --build build
```

### 2. 运行所有测试
```bash
cd build
make run_all_tests
# 或者使用ctest
ctest --output-on-failure
```

### 3. 运行特定类型测试
```bash
# 只运行单元测试
ctest -R unit_tests

# 只运行性能测试  
ctest -R performance_tests

# 只运行覆盖率测试
ctest -R coverage_tests
```

### 4. 覆盖率测试
```bash
# 生成覆盖率报告
cd build
make coverage_tests
gcov ../tests/unit/*.cpp
```

## 配置选项
- `ENABLE_INTEGRATED_TEST_FRAMEWORK`: 启用集成测试框架 (默认: ON)
- `ENABLE_UNIT_TESTS`: 启用单元测试 (默认: ON)
- `ENABLE_COVERAGE_TESTS`: 启用覆盖率测试 (默认: ON)
- `ENABLE_PERFORMANCE_TESTS`: 启用性能测试 (默认: ON)

## 测试结果统计
- **总测试文件**: 70个
- **Unit测试**: 44个 (62.9%)
- **Performance测试**: 25个 (35.7%) 
- **Coverage测试**: 1个 (1.4%)

---
*SQLCC v1.1.1 集成测试框架 - 统一测试管理*
