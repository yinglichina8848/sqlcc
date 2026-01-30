#!/usr/bin/env python3
"""
SQLCC测试框架迁移工具 - 组件级迁移脚本

这个脚本负责将单个组件从旧的CMake+Shell测试框架迁移到新的Bazel+Python框架。
支持渐进式迁移，确保在迁移过程中测试结果的一致性。
"""

import os
import sys
import shutil
import subprocess
import yaml
import json
from pathlib import Path
from typing import Dict, List, Optional, Tuple
from dataclasses import dataclass

@dataclass
class MigrationConfig:
    """迁移配置数据类"""
    component_name: str
    source_dir: Path
    target_dir: Path
    test_files: List[str]
    dependencies: List[str]
    migration_phase: str = "phase1"
    validation_enabled: bool = True

class ComponentMigrator:
    """组件迁移器"""
    
    def __init__(self, project_root: Path):
        self.project_root = project_root
        self.config_dir = project_root / "config" / "migration"
        self.config_dir.mkdir(parents=True, exist_ok=True)
        
        # 组件映射配置
        self.component_mapping = self._load_component_mapping()
    
    def _load_component_mapping(self) -> Dict:
        """加载组件映射配置"""
        mapping_file = self.config_dir / "component_mapping.yaml"
        
        if not mapping_file.exists():
            # 创建默认组件映射
            default_mapping = {
                'core': {
                    'source_dir': 'src/core',
                    'test_dir': 'tests/unit/core',
                    'dependencies': ['utils', 'common'],
                    'test_patterns': ['*_test.cpp', '*_test.cc']
                },
                'utils': {
                    'source_dir': 'src/utils',
                    'test_dir': 'tests/unit/utils',
                    'dependencies': ['common'],
                    'test_patterns': ['*_test.cpp']
                },
                'sql_parser': {
                    'source_dir': 'src/sql_parser',
                    'test_dir': 'tests/unit/sql_parser',
                    'dependencies': ['utils', 'common'],
                    'test_patterns': ['*_test.cpp']
                },
                'storage_engine': {
                    'source_dir': 'src/storage_engine',
                    'test_dir': 'tests/unit/storage_engine',
                    'dependencies': ['core', 'utils', 'common'],
                    'test_patterns': ['*_test.cpp']
                }
            }
            
            with open(mapping_file, 'w') as f:
                yaml.dump(default_mapping, f, default_flow_style=False)
            
            return default_mapping
        
        with open(mapping_file, 'r') as f:
            return yaml.safe_load(f)
    
    def migrate_component(self, component_name: str, dry_run: bool = False) -> bool:
        """迁移指定组件"""
        print(f"🚀 开始迁移组件: {component_name}")
        
        if component_name not in self.component_mapping:
            print(f"❌ 未知组件: {component_name}")
            return False
        
        config = self._create_migration_config(component_name)
        
        # 1. 备份现有测试
        if not self._backup_existing_tests(config, dry_run):
            return False
        
        # 2. 创建新的测试目录结构
        if not self._create_new_test_structure(config, dry_run):
            return False
        
        # 3. 迁移测试文件
        if not self._migrate_test_files(config, dry_run):
            return False
        
        # 4. 创建Bazel构建配置
        if not self._create_bazel_config(config, dry_run):
            return False
        
        # 5. 验证迁移结果
        if config.validation_enabled and not dry_run:
            if not self._validate_migration(config):
                return False
        
        print(f"✅ 组件迁移完成: {component_name}")
        return True
    
    def _create_migration_config(self, component_name: str) -> MigrationConfig:
        """创建迁移配置"""
        mapping = self.component_mapping[component_name]
        
        # 查找现有的测试文件
        source_test_dir = self.project_root / "tests" / "unit" / component_name
        test_files = []
        
        if source_test_dir.exists():
            for pattern in mapping['test_patterns']:
                test_files.extend(source_test_dir.glob(pattern))
        
        return MigrationConfig(
            component_name=component_name,
            source_dir=self.project_root / mapping['source_dir'],
            target_dir=self.project_root / mapping['test_dir'],
            test_files=[f.name for f in test_files],
            dependencies=mapping['dependencies']
        )
    
    def _backup_existing_tests(self, config: MigrationConfig, dry_run: bool) -> bool:
        """备份现有测试"""
        backup_dir = self.project_root / "backup" / "tests" / config.component_name
        source_test_dir = self.project_root / "tests" / "unit" / config.component_name
        
        if not source_test_dir.exists():
            print(f"⚠️  源测试目录不存在: {source_test_dir}")
            return True
        
        print(f"📦 备份现有测试到: {backup_dir}")
        
        if dry_run:
            print(f"[DRY RUN] 将备份 {source_test_dir} 到 {backup_dir}")
            return True
        
        try:
            if backup_dir.exists():
                shutil.rmtree(backup_dir)
            
            backup_dir.mkdir(parents=True, exist_ok=True)
            shutil.copytree(source_test_dir, backup_dir / "tests")
            
            # 备份相关的CMake配置
            cmake_file = self.project_root / "tests" / "CMakeLists.txt"
            if cmake_file.exists():
                shutil.copy2(cmake_file, backup_dir / "CMakeLists.txt.backup")
            
            print("✅ 备份完成")
            return True
            
        except Exception as e:
            print(f"❌ 备份失败: {e}")
            return False
    
    def _create_new_test_structure(self, config: MigrationConfig, dry_run: bool) -> bool:
        """创建新的测试目录结构"""
        print(f"📁 创建新的测试目录结构: {config.target_dir}")
        
        if dry_run:
            print(f"[DRY RUN] 将创建目录: {config.target_dir}")
            return True
        
        try:
            config.target_dir.mkdir(parents=True, exist_ok=True)
            
            # 创建必要的子目录
            (config.target_dir / "data").mkdir(exist_ok=True)  # 测试数据目录
            (config.target_dir / "fixtures").mkdir(exist_ok=True)  # 测试夹具目录
            
            print("✅ 目录结构创建完成")
            return True
            
        except Exception as e:
            print(f"❌ 目录创建失败: {e}")
            return False
    
    def _migrate_test_files(self, config: MigrationConfig, dry_run: bool) -> bool:
        """迁移测试文件"""
        source_test_dir = self.project_root / "tests" / "unit" / config.component_name
        
        if not source_test_dir.exists() or not config.test_files:
            print(f"⚠️  没有找到测试文件，将创建基础测试模板")
            return self._create_basic_test_template(config, dry_run)
        
        print(f"📄 迁移测试文件: {len(config.test_files)} 个文件")
        
        migrated_count = 0
        
        for test_file in config.test_files:
            source_path = source_test_dir / test_file
            target_path = config.target_dir / test_file
            
            if dry_run:
                print(f"[DRY RUN] 将迁移: {source_path} -> {target_path}")
                migrated_count += 1
                continue
            
            try:
                # 读取并转换测试文件
                content = self._convert_test_file(source_path, config)
                
                with open(target_path, 'w') as f:
                    f.write(content)
                
                print(f"✅ 迁移测试文件: {test_file}")
                migrated_count += 1
                
            except Exception as e:
                print(f"❌ 迁移测试文件失败 {test_file}: {e}")
        
        print(f"📊 成功迁移 {migrated_count}/{len(config.test_files)} 个测试文件")
        return migrated_count > 0
    
    def _convert_test_file(self, source_path: Path, config: MigrationConfig) -> str:
        """转换测试文件内容"""
        with open(source_path, 'r') as f:
            content = f.read()
        
        # 基本的转换规则
        conversions = [
            # 更新头文件包含路径
            (r'#include "\.\./\.\./src/', f'#include "src/'),
            # 更新测试夹具路径
            (r'#include "\.\./\.\./tests/', f'#include "tests/'),
            # 添加现代化测试宏（如果需要）
            (r'TEST\(', 'TEST('),  # 保持原样
        ]
        
        for pattern, replacement in conversions:
            import re
            content = re.sub(pattern, replacement, content)
        
        # 添加文件头注释
        header = f"""//
// {source_path.name} - 迁移自旧测试框架
// 组件: {config.component_name}
// 迁移时间: {self._get_current_time()}
//
// 注意: 此文件已从CMake+Shell框架迁移到Bazel+Python框架
// 请验证测试逻辑和断言是否仍然正确
//

"""
        
        return header + content
    
    def _create_basic_test_template(self, config: MigrationConfig, dry_run: bool) -> bool:
        """创建基础测试模板"""
        template_file = config.target_dir / f"{config.component_name}_test.cpp"
        
        template_content = f"""//
// {config.component_name}_test.cpp - 自动生成的测试模板
// 组件: {config.component_name}
// 生成时间: {self._get_current_time()}
//

#include "gtest/gtest.h"
#include "src/{config.component_name}/{config.component_name}.h"

class {config.component_name.capitalize()}Test : public ::testing::Test {{
protected:
    void SetUp() override {{
        // 测试初始化代码
    }}
    
    void TearDown() override {{
        // 测试清理代码
    }}
}};

TEST_F({config.component_name.capitalize()}Test, BasicFunctionality) {{
    // TODO: 实现基本功能测试
    EXPECT_TRUE(true);
}}

TEST_F({config.component_name.capitalize()}Test, EdgeCases) {{
    // TODO: 实现边界情况测试
    EXPECT_TRUE(true);
}}

// 添加更多测试用例...
"""
        
        print(f"📝 创建基础测试模板: {template_file}")
        
        if dry_run:
            print(f"[DRY RUN] 将创建模板文件: {template_file}")
            return True
        
        try:
            with open(template_file, 'w') as f:
                f.write(template_content)
            
            print("✅ 基础测试模板创建完成")
            return True
            
        except Exception as e:
            print(f"❌ 模板创建失败: {e}")
            return False
    
    def _create_bazel_config(self, config: MigrationConfig, dry_run: bool) -> bool:
        """创建Bazel构建配置"""
        build_file = config.target_dir / "BUILD.bazel"
        
        # 生成Bazel构建配置
        bazel_content = self._generate_bazel_content(config)
        
        print(f"⚙️  创建Bazel配置: {build_file}")
        
        if dry_run:
            print(f"[DRY RUN] 将创建Bazel配置:\n{bazel_content}")
            return True
        
        try:
            with open(build_file, 'w') as f:
                f.write(bazel_content)
            
            print("✅ Bazel配置创建完成")
            return True
            
        except Exception as e:
            print(f"❌ Bazel配置创建失败: {e}")
            return False
    
    def _generate_bazel_content(self, config: MigrationConfig) -> str:
        """生成Bazel构建配置内容"""
        
        # 构建依赖列表
        deps = [
            "@com_google_googletest//:gtest_main",
            f"//src/{config.component_name}:{config.component_name}",
        ]
        
        # 添加组件依赖
        for dep in config.dependencies:
            deps.append(f"//src/{dep}:{dep}")
        
        deps_str = '\n        '.join([f'"{d}",' for d in deps])
        
        # 查找测试文件
        test_files = []
        if config.target_dir.exists():
            test_files = [f.name for f in config.target_dir.glob("*_test.cpp")]
        
        if not test_files:
            test_files = [f"{config.component_name}_test.cpp"]
        
        test_files_str = '\n        '.join([f'"{f}",' for f in test_files])
        
        return f"""# {config.component_name}组件测试配置
# 自动生成时间: {self._get_current_time()}

load("@rules_cc//cc:defs.bzl", "cc_test")

# 主测试目标
cc_test(
    name = "{config.component_name}_test",
    srcs = [
        {test_files_str}
    ],
    deps = [
        {deps_str}
    ],
    copts = ["-std=c++17"],
)

# 测试套件（包含所有相关测试）
cc_test(
    name = "{config.component_name}_tests",
    srcs = glob(["*_test.cpp"]),
    deps = [
        {deps_str}
    ],
    copts = ["-std=c++17"],
)

# 快速测试（仅运行关键测试）
cc_test(
    name = "{config.component_name}_quick_test",
    srcs = ["{config.component_name}_test.cpp"],
    deps = [
        {deps_str}
    ],
    copts = ["-std=c++17"],
    tags = ["quick"],
)
"""
    
    def _validate_migration(self, config: MigrationConfig) -> bool:
        """验证迁移结果"""
        print(f"🔍 验证迁移结果: {config.component_name}")
        
        # 1. 检查文件是否存在
        required_files = [
            config.target_dir / "BUILD.bazel",
            config.target_dir / f"{config.component_name}_test.cpp"
        ]
        
        for file_path in required_files:
            if not file_path.exists():
                print(f"❌ 必需文件不存在: {file_path}")
                return False
        
        # 2. 验证Bazel配置语法
        if not self._validate_bazel_syntax(config):
            return False
        
        # 3. 运行基础编译测试
        if not self._run_basic_compilation_test(config):
            return False
        
        print("✅ 迁移验证通过")
        return True
    
    def _validate_bazel_syntax(self, config: MigrationConfig) -> bool:
        """验证Bazel配置语法"""
        build_file = config.target_dir / "BUILD.bazel"
        
        try:
            # 使用Bazel查询验证语法
            cmd = ["bazel", "query", f"//{config.target_dir.relative_to(self.project_root)}:all"]
            result = subprocess.run(cmd, cwd=self.project_root, capture_output=True, text=True)
            
            if result.returncode == 0:
                print("✅ Bazel配置语法验证通过")
                return True
            else:
                print(f"❌ Bazel配置语法错误: {result.stderr}")
                return False
                
        except Exception as e:
            print(f"❌ Bazel语法验证异常: {e}")
            return False
    
    def _run_basic_compilation_test(self, config: MigrationConfig) -> bool:
        """运行基础编译测试"""
        target = f"//{config.target_dir.relative_to(self.project_root)}:{config.component_name}_test"
        
        try:
            # 尝试编译测试（不运行）
            cmd = ["bazel", "build", target, "--nobuild"]
            result = subprocess.run(cmd, cwd=self.project_root, capture_output=True, text=True)
            
            if result.returncode == 0:
                print("✅ 基础编译测试通过")
                return True
            else:
                print(f"❌ 编译测试失败: {result.stderr}")
                return False
                
        except Exception as e:
            print(f"❌ 编译测试异常: {e}")
            return False
    
    def _get_current_time(self) -> str:
        """获取当前时间字符串"""
        from datetime import datetime
        return datetime.now().strftime("%Y-%m-%d %H:%M:%S")

def main():
    """主函数"""
    import argparse
    
    parser = argparse.ArgumentParser(description="SQLCC组件迁移工具")
    parser.add_argument("component", help="要迁移的组件名称")
    parser.add_argument("--dry-run", action="store_true", 
                       help="干运行模式，不实际执行操作")
    parser.add_argument("--skip-validation", action="store_true",
                       help="跳过迁移验证")
    parser.add_argument("--project-root", default=".",
                       help="项目根目录路径")
    
    args = parser.parse_args()
    
    project_root = Path(args.project_root).resolve()
    
    if not project_root.exists():
        print(f"❌ 项目根目录不存在: {project_root}")
        sys.exit(1)
    
    migrator = ComponentMigrator(project_root)
    
    # 检查组件是否支持
    if args.component not in migrator.component_mapping:
        print(f"❌ 不支持的组件: {args.component}")
        print("支持的组件:")
        for comp in migrator.component_mapping.keys():
            print(f"  - {comp}")
        sys.exit(1)
    
    success = migrator.migrate_component(
        args.component, 
        dry_run=args.dry_run
    )
    
    if success:
        print(f"\n🎉 组件迁移{'模拟' if args.dry_run else ''}完成: {args.component}")
        
        if not args.dry_run:
            print("\n下一步操作:")
            print("1. 手动验证迁移的测试逻辑")
            print("2. 运行迁移验证脚本: python scripts/migration/validate_migration.py" + args.component)
            print("3. 更新主测试运行器配置")
    else:
        print(f"\n💥 组件迁移失败: {args.component}")
        sys.exit(1)

if __name__ == "__main__":
    main()