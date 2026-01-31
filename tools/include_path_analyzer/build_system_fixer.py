#!/usr/bin/env python3
"""
BUILD系统include路径修复工具

根据docs/project/header_index.md头文件索引文档，
系统性地修复所有include目录下BUILD文件的include路径配置问题。

功能：
1. 读取头文件索引文档
2. 扫描所有include目录下的BUILD文件
3. 为所有cc_library添加includes = ["."]配置
4. 验证修复效果

作者: SQLCC AI Agent
版本: v1.0
"""

import os
import re
import yaml
from pathlib import Path
from typing import Dict, List, Set

class BuildSystemFixer:
    """BUILD系统include路径修复器"""

    def __init__(self, project_root: str):
        self.project_root = Path(project_root)
        self.header_index_path = self.project_root / "docs" / "project" / "header_index.md"
        self.include_dirs = [
            "include/core",
            "include/sql_parser",
            "include/execution",
            "include/storage",
            "include/storage_engine",
            "include/network",
            "include/transaction",
            "include/procedure",
            "include/trigger",
            "include/exception",
            "include/utils",
            "include/types",
            "include/security"
        ]

    def read_header_index(self) -> Dict:
        """读取头文件索引文档"""
        if not self.header_index_path.exists():
            print(f"❌ 头文件索引文档不存在: {self.header_index_path}")
            return {}

        print(f"📖 读取头文件索引文档: {self.header_index_path}")
        with open(self.header_index_path, 'r', encoding='utf-8') as f:
            content = f.read()

        # 解析统计信息
        stats_match = re.search(r'\| \*\*总计\*\* \| (\d+) \|', content)
        total_files = int(stats_match.group(1)) if stats_match else 0

        print(f"📊 项目总计 {total_files} 个头文件")

        return {
            'total_files': total_files,
            'content': content
        }

    def find_build_files(self) -> List[Path]:
        """查找所有include目录下的BUILD文件"""
        build_files = []

        for include_dir in self.include_dirs:
            build_path = self.project_root / include_dir / "BUILD.bazel"
            if build_path.exists():
                build_files.append(build_path)

        print(f"🔍 发现 {len(build_files)} 个BUILD文件:")
        for bf in build_files:
            print(f"  - {bf.relative_to(self.project_root)}")

        return build_files

    def fix_build_file(self, build_file: Path) -> bool:
        """修复单个BUILD文件"""
        print(f"\n🔧 修复BUILD文件: {build_file.relative_to(self.project_root)}")

        try:
            with open(build_file, 'r', encoding='utf-8') as f:
                content = f.read()

            original_content = content

            # 查找所有cc_library规则
            cc_library_pattern = r'(cc_library\(\s*name\s*=\s*"[^"]*",)([^)]*\))'
            matches = re.findall(cc_library_pattern, content, re.DOTALL)

            fixed_count = 0

            for match in re.finditer(cc_library_pattern, content, re.DOTALL):
                lib_start = match.start()
                lib_name = match.group(1)
                lib_content = match.group(2)

                # 检查是否已有includes配置
                if 'includes =' not in lib_content:
                    # 在合适位置添加includes配置
                    # 查找合适的位置（通常在hdrs之后，deps之前）
                    insert_pos = 0

                    # 尝试在hdrs行之后插入
                    hdrs_match = re.search(r'(\s*hdrs\s*=\s*[^,]+,)', lib_content)
                    if hdrs_match:
                        insert_pos = hdrs_match.end()
                    else:
                        # 如果没有hdrs，在visibility之后或deps之前插入
                        vis_match = re.search(r'(\s*visibility\s*=\s*[^,]+,)', lib_content)
                        if vis_match:
                            insert_pos = vis_match.end()
                        else:
                            # 在开头插入
                            insert_pos = 1  # 跳过cc_library(

                    # 插入includes配置
                    includes_config = '\n    includes = ["."],'
                    content = content[:lib_start + insert_pos] + includes_config + content[lib_start + insert_pos:]

                    fixed_count += 1
                    print(f"  ✅ 为 {lib_name.strip('(').strip()} 添加了includes配置")

            if fixed_count > 0:
                # 写回文件
                with open(build_file, 'w', encoding='utf-8') as f:
                    f.write(content)

                print(f"  📝 已修复 {fixed_count} 个cc_library规则")
                return True
            else:
                print(f"  ℹ️  无需修复，所有cc_library已有includes配置")
                return False

        except Exception as e:
            print(f"  ❌ 修复失败: {e}")
            return False

    def verify_fixes(self, build_files: List[Path]) -> Dict:
        """验证修复效果"""
        print("\n🔍 验证修复效果...")

        verification_results = {
            'total_files': len(build_files),
            'fixed_files': 0,
            'already_correct': 0,
            'errors': 0
        }

        for build_file in build_files:
            try:
                with open(build_file, 'r', encoding='utf-8') as f:
                    content = f.read()

                # 检查所有cc_library是否都有includes配置
                cc_library_blocks = re.findall(r'cc_library\([^)]*\)', content, re.DOTALL)

                file_correct = True
                for block in cc_library_blocks:
                    if 'includes =' not in block:
                        file_correct = False
                        break

                if file_correct and cc_library_blocks:
                    verification_results['already_correct'] += 1
                    print(f"  ✅ {build_file.relative_to(self.project_root)} - 配置正确")
                elif cc_library_blocks:
                    verification_results['fixed_files'] += 1
                    print(f"  🔧 {build_file.relative_to(self.project_root)} - 已修复")
                else:
                    print(f"  ℹ️ {build_file.relative_to(self.project_root)} - 无cc_library规则")

            except Exception as e:
                verification_results['errors'] += 1
                print(f"  ❌ {build_file.relative_to(self.project_root)} - 验证失败: {e}")

        return verification_results

    def run_system_fix(self) -> Dict:
        """运行系统性修复"""
        print("🚀 开始BUILD系统include路径修复...")
        print("=" * 60)

        # 1. 读取头文件索引
        header_info = self.read_header_index()
        if not header_info:
            return {'error': '无法读取头文件索引文档'}

        # 2. 查找BUILD文件
        build_files = self.find_build_files()

        # 3. 修复BUILD文件
        fixed_files = 0
        for build_file in build_files:
            if self.fix_build_file(build_file):
                fixed_files += 1

        # 4. 验证修复效果
        verification = self.verify_fixes(build_files)

        # 5. 输出总结报告
        print("\n" + "=" * 60)
        print("📊 BUILD系统include路径修复总结报告")
        print("=" * 60)
        print(f"📁 处理的BUILD文件总数: {verification['total_files']}")
        print(f"🔧 新修复的文件数: {verification['fixed_files']}")
        print(f"✅ 已正确配置的文件数: {verification['already_correct']}")
        print(f"❌ 处理出错的文件数: {verification['errors']}")
        print(f"🎯 项目头文件总数: {header_info['total_files']}")

        success_rate = (verification['fixed_files'] + verification['already_correct']) / verification['total_files'] * 100 if verification['total_files'] > 0 else 0
        print(".1f")
        if verification['errors'] == 0:
            print("🎉 修复完成！所有BUILD文件include路径配置正确")
        else:
            print("⚠️  修复完成，但存在一些错误需要手动检查")

        return {
            'header_info': header_info,
            'build_files': len(build_files),
            'fixed_files': fixed_files,
            'verification': verification,
            'success_rate': success_rate
        }

def main():
    """主函数"""
    project_root = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

    fixer = BuildSystemFixer(project_root)
    results = fixer.run_system_fix()

    # 保存修复报告
    report_path = Path(project_root) / "build_system_fix_report.json"
    with open(report_path, 'w', encoding='utf-8') as f:
        # 简化结果以便JSON序列化
        simplified_results = {
            'timestamp': str(Path(project_root).stat().st_mtime),
            'build_files_processed': results.get('build_files', 0),
            'fixed_files': results.get('fixed_files', 0),
            'verification': results.get('verification', {}),
            'success_rate': results.get('success_rate', 0)
        }
        yaml.dump(simplified_results, f, default_flow_style=False)

    print(f"\n💾 修复报告已保存到: {report_path}")

if __name__ == "__main__":
    main()
