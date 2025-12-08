# SQLCC 现代化测试框架设计方案

## 问题分析

### 当前测试框架的痛点

1. **CMake配置复杂**
   - 每个测试都需要重复的编译配置
   - 参数设置容易出错
   - 依赖管理不够直观

2. **Shell脚本维护困难**
   - 脚本逻辑分散，难以统一管理
   - 错误处理机制不完善
   - 跨平台兼容性差

3. **缺乏分层测试架构**
   - 测试范围定义不清晰
   - 组件间测试依赖关系复杂
   - 难以实现增量测试

## 现代化测试框架设计

### 设计理念

借鉴Maven+Jacoco的思想，但针对C++项目特点进行优化：

- **声明式配置**：通过配置文件定义测试，而非命令式脚本
- **依赖管理**：自动处理测试依赖关系
- **分层测试**：支持组件级、模块级、系统级测试
- **统一接口**：提供一致的测试执行和报告接口

### 技术栈选择

#### 核心框架
- **构建工具**: Bazel 或 Meson（替代CMake）
- **测试框架**: Google Test + Google Benchmark
- **覆盖率工具**: gcovr + lcov（替代Jacoco）
- **配置管理**: YAML/TOML（替代Shell脚本）
- **脚本语言**: Python 3.x（替代Bash）

#### 可选方案对比

| 方案 | 优势 | 劣势 | 推荐度 |
|------|------|------|--------|
| Bazel + Python | 强大的依赖管理，增量构建 | 学习曲线较陡 | ⭐⭐⭐⭐⭐ |
| Meson + Python | 配置简单，性能优秀 | 生态相对较小 | ⭐⭐⭐⭐ |
| CMake改进版 | 迁移成本低 | 无法根本解决问题 | ⭐⭐⭐ |

## 详细设计方案

### 1. 项目结构重构

```
sqlcc/
├── WORKSPACE              # Bazel工作空间配置
├── BUILD.bazel            # 根构建配置
├── test_config.yaml       # 测试配置文件
├── src/                   # 源代码
├── tests/                 # 测试代码
│   ├── unit/             # 单元测试
│   ├── integration/      # 集成测试
│   ├── performance/      # 性能测试
│   └── e2e/              # 端到端测试
├── tools/                # 测试工具
│   ├── test_runner.py    # 测试执行器
│   ├── coverage_tool.py  # 覆盖率工具
│   └── report_generator.py # 报告生成器
└── config/               # 配置文件
    ├── test_profiles/    # 测试配置文件
    └── dependencies/     # 依赖配置
```

### 2. Bazel构建配置示例

#### WORKSPACE文件
```python
workspace(name = "sqlcc")

# 加载外部依赖
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# Google Test依赖
http_archive(
    name = "com_google_googletest",
    urls = ["https://github.com/google/googletest/archive/v1.14.0.zip"],
    strip_prefix = "googletest-1.14.0",
)

# Google Benchmark依赖
http_archive(
    name = "com_google_benchmark",
    urls = ["https://github.com/google/benchmark/archive/v1.8.0.zip"],
    strip_prefix = "benchmark-1.8.0",
)
```

#### BUILD.bazel文件
```python
# 定义测试套件
test_suite(
    name = "sqlcc_tests",
    tests = [
        "//tests/unit:all",
        "//tests/integration:all",
        "//tests/performance:all",
    ],
)

# 覆盖率配置
config_setting(
    name = "coverage",
    values = {"define": "enable_coverage=true"},
)
```

### 3. 测试配置文件（YAML格式）

#### test_config.yaml
```yaml
# SQLCC测试配置
version: "1.0"
project:
  name: "SQLCC"
  version: "1.1.1"

test_profiles:
  quick:
    description: "快速验证测试"
    includes:
      - "unit/core"
      - "unit/basic"
    excludes:
      - "performance"
      - "integration/complex"
    timeout: 300
    
  full:
    description: "完整测试套件"
    includes:
      - "unit"
      - "integration"
      - "performance"
    timeout: 1800
    
  coverage:
    description: "覆盖率测试"
    includes:
      - "unit"
      - "integration"
    coverage_enabled: true
    timeout: 2400

components:
  sql_parser:
    description: "SQL解析器组件"
    test_suites:
      - "unit/sql_parser"
      - "integration/parser"
    dependencies:
      - "core"
      - "utils"
      
  storage_engine:
    description: "存储引擎组件"
    test_suites:
      - "unit/storage"
      - "integration/storage"
      - "performance/crud"
    dependencies:
      - "core"
      - "buffer_pool"

reporting:
  formats:
    - "html"
    - "json"
    - "junit"
  output_dir: "test_reports"
  
coverage:
  tool: "gcovr"
  output_formats:
    - "html"
    - "xml"
  thresholds:
    line: 80
    branch: 70
    function: 85
```

### 4. Python测试执行器

#### test_runner.py
```python
#!/usr/bin/env python3
"""
SQLCC现代化测试执行器
基于Bazel和配置驱动的测试框架
"""

import yaml
import subprocess
import sys
import os
from pathlib import Path
from typing import Dict, List, Optional
from dataclasses import dataclass

@dataclass
class TestConfig:
    """测试配置数据类"""
    profile: str
    includes: List[str]
    excludes: List[str]
    timeout: int
    coverage_enabled: bool = False

class TestRunner:
    """测试执行器主类"""
    
    def __init__(self, config_path: str = "test_config.yaml"):
        self.config = self._load_config(config_path)
        self.project_root = Path(__file__).parent.parent
        
    def _load_config(self, config_path: str) -> Dict:
        """加载测试配置"""
        with open(config_path, 'r') as f:
            return yaml.safe_load(f)
    
    def get_test_profile(self, profile_name: str) -> TestConfig:
        """获取测试配置"""
        profile_data = self.config['test_profiles'][profile_name]
        return TestConfig(
            profile=profile_name,
            includes=profile_data.get('includes', []),
            excludes=profile_data.get('excludes', []),
            timeout=profile_data.get('timeout', 600),
            coverage_enabled=profile_data.get('coverage_enabled', False)
        )
    
    def run_tests(self, profile: str, component: Optional[str] = None) -> bool:
        """执行测试"""
        test_config = self.get_test_profile(profile)
        
        # 构建测试目标
        targets = self._build_test_targets(test_config, component)
        
        if not targets:
            print("❌ 没有找到匹配的测试目标")
            return False
        
        # 执行Bazel测试命令
        bazel_args = ["bazel", "test"] + targets
        
        if test_config.coverage_enabled:
            bazel_args.extend(["--define", "enable_coverage=true"])
        
        bazel_args.extend([
            "--test_timeout", str(test_config.timeout),
            "--test_output=all",
            "--cache_test_results=no"
        ])
        
        print(f"🚀 执行测试配置: {profile}")
        print(f"📋 测试目标: {', '.join(targets)}")
        
        try:
            result = subprocess.run(bazel_args, cwd=self.project_root, check=True)
            return result.returncode == 0
        except subprocess.CalledProcessError as e:
            print(f"❌ 测试执行失败: {e}")
            return False
    
    def _build_test_targets(self, config: TestConfig, component: Optional[str]) -> List[str]:
        """构建测试目标列表"""
        targets = []
        
        if component:
            # 组件级测试
            component_config = self.config['components'].get(component)
            if not component_config:
                raise ValueError(f"未知组件: {component}")
            
            targets.extend([f"//tests/{suite}:all" for suite in component_config['test_suites']])
        else:
            # 配置级测试
            for include_pattern in config.includes:
                targets.append(f"//tests/{include_pattern}:all")
        
        # 应用排除规则
        final_targets = []
        for target in targets:
            if not any(exclude in target for exclude in config.excludes):
                final_targets.append(target)
        
        return final_targets
    
    def generate_coverage_report(self) -> bool:
        """生成覆盖率报告"""
        try:
            # 使用gcovr生成覆盖率报告
            cmd = [
                "gcovr",
                "--root", str(self.project_root),
                "--exclude", "tests/.*",
                "--html", "--html-detail",
                "--output", "coverage_report.html"
            ]
            
            result = subprocess.run(cmd, cwd=self.project_root, check=True)
            
            if result.returncode == 0:
                print("✅ 覆盖率报告生成成功")
                return True
            else:
                print("❌ 覆盖率报告生成失败")
                return False
                
        except Exception as e:
            print(f"❌ 覆盖率报告生成错误: {e}")
            return False

def main():
    """主函数"""
    import argparse
    
    parser = argparse.ArgumentParser(description="SQLCC测试执行器")
    parser.add_argument("profile", choices=["quick", "full", "coverage"], 
                       help="测试配置")
    parser.add_argument("--component", help="指定测试组件")
    parser.add_argument("--coverage", action="store_true", 
                       help="生成覆盖率报告")
    
    args = parser.parse_args()
    
    runner = TestRunner()
    
    # 执行测试
    success = runner.run_tests(args.profile, args.component)
    
    # 生成覆盖率报告
    if args.coverage or args.profile == "coverage":
        runner.generate_coverage_report()
    
    sys.exit(0 if success else 1)

if __name__ == "__main__":
    main()
```

### 5. 使用示例

#### 快速测试
```bash
# 运行快速测试
python tools/test_runner.py quick

# 运行特定组件测试
python tools/test_runner.py quick --component sql_parser

# 运行完整测试并生成覆盖率报告
python tools/test_runner.py coverage --coverage
```

#### Bazel原生命令
```bash
# 运行所有测试
bazel test //tests/...

# 运行特定测试套件
bazel test //tests/unit:all

# 生成覆盖率报告
bazel coverage //tests/... --define enable_coverage=true
```

## 迁移计划

### 阶段一：基础设施准备（1-2周）
1. 安装Bazel构建工具
2. 创建新的项目结构
3. 编写基础配置文件和工具

### 阶段二：核心组件迁移（2-3周）
1. 迁移核心测试用例到新框架
2. 验证测试执行正确性
3. 集成覆盖率工具

### 阶段三：全面迁移（1-2周）
1. 迁移所有剩余测试用例
2. 优化测试配置和参数
3. 更新文档和CI/CD配置

### 阶段四：优化和完善（持续）
1. 性能优化和并行测试
2. 添加高级测试功能
3. 完善监控和报告功能

## 优势对比

### 新框架 vs 旧框架

| 特性 | 旧框架 (CMake+Shell) | 新框架 (Bazel+Python) |
|------|---------------------|----------------------|
| 配置复杂度 | 高（每个测试独立配置） | 低（声明式配置） |
| 依赖管理 | 手动管理 | 自动依赖解析 |
| 构建性能 | 慢（全量构建） | 快（增量构建） |
| 跨平台支持 | 有限 | 优秀 |
| 维护成本 | 高 | 低 |
| 测试分层 | 困难 | 天然支持 |

## 开源替代方案调研

### 成熟的C++测试框架

1. **CTest + CDash**
   - 优势：CMake原生支持，成熟稳定
   - 劣势：仍然依赖CMake，配置复杂

2. **Catch2**
   - 优势：现代C++测试框架，配置简单
   - 劣势：需要迁移现有Google Test用例

3. **doctest**
   - 优势：轻量级，编译速度快
   - 劣势：功能相对简单

4. **Bazel + Google Test**（推荐）
   - 优势：工业级质量，强大的依赖管理
   - 劣势：学习曲线较陡

## 实施建议

### 短期改进（立即开始）
1. **逐步引入Python脚本**：将复杂的Shell逻辑迁移到Python
2. **统一配置管理**：创建YAML配置文件管理测试参数
3. **优化现有CMake**：简化重复的测试配置

### 中期迁移（1-2个月）
1. **评估Bazel可行性**：在小范围试点Bazel构建
2. **分组件迁移**：按优先级逐步迁移测试用例
3. **并行运行**：新旧框架并行运行确保稳定性

### 长期目标（3-6个月）
1. **完全迁移到新框架**：淘汰旧的CMake+Shell框架
2. **实现高级功能**：智能测试选择、性能监控等
3. **开源贡献**：将改进的框架贡献给社区

## 总结

新的测试框架设计解决了当前CMake+Shell框架的主要痛点，提供了：

1. **声明式配置**：通过YAML文件定义测试，减少重复代码
2. **强大的依赖管理**：Bazel自动处理复杂的依赖关系
3. **分层测试支持**：天然支持组件级、模块级测试
4. **现代化工具链**：Python脚本提供更好的维护性和跨平台支持
5. **企业级质量**：基于工业标准的工具和最佳实践

这个设计方案将为SQLCC项目提供一个可持续、易维护的现代化测试体系。

---

*本文档最后更新: 2025年12月*  
*设计者: SQLCC开发团队*  
*版本: v1.0*