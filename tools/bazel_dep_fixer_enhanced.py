#!/usr/bin/env python3
"""
增强版Bazel依赖修复工具

自动修复Bazel BUILD文件中的依赖问题：
- 修复缺失的依赖
- 移除未使用的依赖
- 优化依赖声明
- 修复相对路径依赖问题
"""

import re
import sys
import subprocess
from pathlib import Path
from typing import List, Set, Dict, Optional


class EnhancedBazelDepFixer:
    """增强版Bazel依赖修复器"""

    def __init__(self, project_root: str = "."):
        self.project_root = Path(project_root).resolve()
        self.fixed_count = 0
        self.errors = []
        self.dep_mapping = {
            # 基于include文件推断依赖
            'sql_parser': '//src/sql_parser:sql_parser',
            'storage': '//src/storage_engine:storage_engine',
            'core': '//src/core:core',
            'utils': '//src/utils:utils',
            'network': '//src/network:network',
            'execution': '//src/execution:execution',
            'transaction': '//src/transaction:transaction',
            'types': '//src/types:types',
            'procedure': '//src/procedure:procedure',
            'trigger': '//src/trigger:trigger',
            'gtest': '@com_google_googletest//:gtest_main',
            'gmock': '@com_google_googletest//:gtest_main'
        }

    def fix_file(self, file_path: str, dry_run: bool = False) -> bool:
        """修复单个BUILD文件"""
        build_path = Path(file_path)
        if not build_path.exists():
            self.errors.append(f"文件不存在: {file_path}")
            return False

        try:
            with open(build_path, 'r', encoding='utf-8') as f:
                content = f.read()
        except UnicodeDecodeError:
            self.errors.append(f"无法读取文件 (编码问题): {file_path}")
            return False

        original_content = content

        # 修复依赖问题
        content = self._fix_deps(content, build_path)

        # 修复相对路径依赖
        content = self._fix_relative_deps(content)

        # 优化依赖声明顺序
        content = self._optimize_deps_order(content)

        if content != original_content:
            if not dry_run:
                try:
                    with open(build_path, 'w', encoding='utf-8') as f:
                        f.write(content)
                    print(f"✅ 已修复: {file_path}")
                except IOError as e:
                    self.errors.append(f"无法写入文件 {file_path}: {e}")
                    return False
            else:
                print(f"🔍 [DRY RUN] 需要修复: {file_path}")

            self.fixed_count += 1
            return True
        else:
            if dry_run:
                print(f"✅ 无需修复: {file_path}")
            return True

    def _fix_deps(self, content: str, build_path: Path) -> str:
        """修复依赖声明"""
        # 查找deps块
        deps_match = re.search(r'deps\s*=\s*\[(.*?)\]', content, re.DOTALL)
        if not deps_match:
            return content

        deps_block = deps_match.group(1)
        current_deps = self._parse_deps(deps_block)
        
        # 推断所需的依赖
        required_deps = self._infer_required_deps(build_path)
        
        # 合并现有依赖和所需依赖
        all_deps = set(current_deps) | set(required_deps)
        
        # 移除明显不需要的依赖
        filtered_deps = self._filter_unnecessary_deps(list(all_deps), build_path)
        
        # 重新排序依赖
        sorted_deps = self._sort_deps(filtered_deps)
        
        # 重新构造deps块
        if sorted_deps:
            new_deps_block = ',\n        '.join([f'"{dep}"' for dep in sorted_deps])
            new_deps_section = f'deps = [\n        {new_deps_block},\n    ]'
            content = content.replace(deps_match.group(0), new_deps_section)
        
        return content

    def _fix_relative_deps(self, content: str) -> str:
        """修复相对路径依赖"""
        # 修复常见的相对路径依赖问题
        # ../../src/xxx -> //src/xxx:xxx
        content = re.sub(
            r'"../../src/([^/]+)/([^/]+)"', 
            r'"//src/\1:\2"', 
            content
        )
        
        # ../../../src/xxx -> //src/xxx:xxx
        content = re.sub(
            r'"../../../src/([^/]+)/([^/]+)"', 
            r'"//src/\1:\2"', 
            content
        )
        
        # ../../include -> //include
        content = re.sub(
            r'"../../include"', 
            r'"//include"', 
            content
        )
        
        # ../../../include -> //include
        content = re.sub(
            r'"../../../include"', 
            r'"//include"', 
            content
        )
        
        return content

    def _optimize_deps_order(self, content: str) -> str:
        """优化依赖声明顺序"""
        deps_match = re.search(r'deps\s*=\s*\[(.*?)\]', content, re.DOTALL)
        if not deps_match:
            return content

        deps_block = deps_match.group(1)
        deps = self._parse_deps(deps_block)
        
        # 按照推荐顺序排序依赖
        sorted_deps = self._sort_deps(deps)
        
        if sorted_deps:
            new_deps_block = ',\n        '.join([f'"{dep}"' for dep in sorted_deps])
            new_deps_section = f'deps = [\n        {new_deps_block},\n    ]'
            content = content.replace(deps_match.group(0), new_deps_section)
        
        return content

    def _parse_deps(self, deps_block: str) -> List[str]:
        """解析依赖列表"""
        # 提取引号内的依赖
        deps = re.findall(r'"([^"]+)"', deps_block)
        return deps

    def _infer_required_deps(self, build_path: Path) -> List[str]:
        """推断所需的依赖"""
        required_deps = []
        
        # 如果是测试文件，添加gtest依赖
        if 'test' in build_path.name.lower():
            required_deps.append('@com_google_googletest//:gtest_main')
        
        # 查找相关的源文件来推断依赖
        parent_dir = build_path.parent
        source_files = list(parent_dir.glob("*.cpp")) + list(parent_dir.glob("*.cc"))
        
        # 分析源文件中的include语句
        for source_file in source_files:
            if source_file.exists():
                try:
                    with open(source_file, 'r', encoding='utf-8') as f:
                        content = f.read()
                    
                    includes = re.findall(r'#include\s*[<"]([^>"]+)[>"]', content)
                    for include in includes:
                        # 基于include推断依赖
                        for module, dep in self.dep_mapping.items():
                            if module in include and dep not in required_deps:
                                required_deps.append(dep)
                                break
                except:
                    continue
        
        return required_deps

    def _filter_unnecessary_deps(self, deps: List[str], build_path: Path) -> List[str]:
        """过滤不必要的依赖"""
        # 移除明显不相关的依赖
        filtered_deps = []
        for dep in deps:
            # 保留有效的依赖
            if (dep.startswith('//') or dep.startswith('@')) and len(dep) > 2:
                filtered_deps.append(dep)
        
        return filtered_deps

    def _sort_deps(self, deps: List[str]) -> List[str]:
        """按推荐顺序排序依赖"""
        # 定义依赖优先级
        priority_order = [
            '@com_google_googletest',  # 测试框架优先
            '//src/core',              # 核心模块
            '//src/utils',             # 工具模块
            '//src/network',           # 网络模块
            '//src/storage_engine',    # 存储引擎
            '//src/sql_parser',        # SQL解析器
            '//src/execution',         # 执行器
            '//src/transaction',       # 事务管理
            '//src/types',             # 类型系统
            '//src/procedure',         # 存储过程
            '//src/trigger',           # 触发器
            '//src',                   # 其他源码
            '//include',               # 头文件
        ]
        
        # 按优先级排序
        sorted_deps = []
        remaining_deps = deps[:]
        
        for prefix in priority_order:
            matched_deps = [dep for dep in remaining_deps if dep.startswith(prefix)]
            matched_deps.sort()
            sorted_deps.extend(matched_deps)
            remaining_deps = [dep for dep in remaining_deps if not dep.startswith(prefix)]
        
        # 添加剩余的依赖
        remaining_deps.sort()
        sorted_deps.extend(remaining_deps)
        
        return sorted_deps

    def fix_directory(self, dir_path: str, dry_run: bool = False) -> bool:
        """修复目录中的所有BUILD文件"""
        build_files = Path(dir_path).rglob("BUILD.bazel")
        build_files = list(build_files) + list(Path(dir_path).rglob("BUILD"))

        success = True
        for build_file in build_files:
            if not self.fix_file(str(build_file), dry_run):
                success = False

        return success

    def report(self):
        """生成报告"""
        if self.errors:
            print("❌ 修复过程中出现错误:")
            for error in self.errors:
                print(f"  - {error}")

        print(f"📊 修复统计: 修复了 {self.fixed_count} 个文件")

        if not self.errors and self.fixed_count > 0:
            print("✅ 依赖修复完成")
            return True
        elif not self.errors and self.fixed_count == 0:
            print("✅ 所有依赖都已正确")
            return True
        else:
            print("❌ 修复过程中出现错误")
            return False


def main():
    if len(sys.argv) < 2:
        print("用法: python bazel_dep_fixer_enhanced.py <BUILD文件或目录> [--dry-run]")
        print("选项:")
        print("  --dry-run  只显示需要修复的文件，不实际修改")
        sys.exit(1)

    path = sys.argv[1]
    dry_run = "--dry-run" in sys.argv

    fixer = EnhancedBazelDepFixer()

    if Path(path).is_file():
        # 修复单个文件
        success = fixer.fix_file(path, dry_run)
    elif Path(path).is_dir():
        # 修复目录中的所有BUILD文件
        success = fixer.fix_directory(path, dry_run)
    else:
        print(f"路径不存在: {path}")
        sys.exit(1)

    success = fixer.report()
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()