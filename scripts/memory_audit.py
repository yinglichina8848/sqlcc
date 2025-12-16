#!/usr/bin/env python3
"""
SQLCC 内存安全审计工具
分析核心源代码文件中的内存安全问题
"""

import os
import re
import sys
from collections import defaultdict
from datetime import datetime
from pathlib import Path

class MemoryAuditor:
    def __init__(self):
        self.issues = []
        self.file_issues = defaultdict(list)
        
        # 内存安全问题模式
        self.patterns = {
            'raw_new_delete': r'\b(new|delete)\s+[^\s;)]+',
            'raw_malloc_free': r'\b(malloc|free|calloc|realloc)\s*\(',
            'raw_ptr_dereference': r'->[a-zA-Z_][a-zA-Z0-9_]*',
            'potential_memory_leak': r'new\s+[^\s;)]+[^;]*;',
            'double_delete': r'delete\s+[^\s;)]+[^;]*;[^}]*delete\s+[^\s;)]+[^;]*;',
            'null_dereference': r'->[a-zA-Z_][a-zA-Z0-9_]*\s*\(',
            'uninitialized_pointer': r'[a-zA-Z_][a-zA-Z0-9_]*\s*\*\s*[a-zA-Z_][a-zA-Z0-9_]*\s*=\s*(?!nullptr|NULL|0)',
            'missing_delete': r'new\s+[^\s;)]+[^;]*;[^}]*return[^}]*;',
            'c_style_cast': r'\([a-zA-Z_][a-zA-Z0-9_]*\s*\*\s*\)',
            'void_pointer_arithmetic': r'\([^\)]*\)\s*[\+\-]\s*\d+',
            'buffer_overflow': r'\[[^\]]*\]\s*=\s*[^;]*;',
            'stack_overflow': r'static\s+[a-zA-Z_][a-zA-Z0-9_]*\s+[a-zA-Z_][a-zA-Z0-9_]*\s*\[\s*\d+\s*\]\s*;'
        }
        
        # 需要特别检查的目录和文件
        self.target_dirs = [
            'src/storage_engine',
            'src/network', 
            'src/core',
            'src/execution',
            'src/sql_parser',
            'include'
        ]
        
    def analyze_file(self, filepath):
        """分析单个文件的内存安全问题"""
        try:
            with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()

            # 预处理：识别智能指针变量
            smart_pointer_vars = self._identify_smart_pointers(content)

            lines = content.split('\n')

            for line_num, line in enumerate(lines, 1):
                line = line.strip()
                if not line or line.startswith('//') or line.startswith('/*'):
                    continue

                for issue_type, pattern in self.patterns.items():
                    matches = re.finditer(pattern, line)
                    for match in matches:
                        # 检查是否是智能指针的误报
                        if self._is_smart_pointer_false_positive(issue_type, match.group(), line, smart_pointer_vars):
                            continue

                        issue = {
                            'file': str(filepath),
                            'line': line_num,
                            'type': issue_type,
                            'content': line,
                            'match': match.group(),
                            'severity': self._get_severity(issue_type, line)
                        }
                        self.issues.append(issue)
                        self.file_issues[str(filepath)].append(issue)

        except Exception as e:
            print(f"Error analyzing {filepath}: {e}")

    def _identify_smart_pointers(self, content):
        """识别文件中的智能指针变量"""
        smart_pointer_vars = set()

        # 匹配智能指针声明模式
        patterns = [
            r'std::shared_ptr<[^>]+>\s+([a-zA-Z_][a-zA-Z0-9_]*)',  # std::shared_ptr<Type> var
            r'std::unique_ptr<[^>]+>\s+([a-zA-Z_][a-zA-Z0-9_]*)',  # std::unique_ptr<Type> var
            r'auto\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*=',  # auto var = ...
        ]

        for pattern in patterns:
            matches = re.finditer(pattern, content)
            for match in matches:
                var_name = match.group(1)
                smart_pointer_vars.add(var_name)

        return smart_pointer_vars

    def _is_smart_pointer_false_positive(self, issue_type, match_text, line, smart_pointer_vars):
        """检查是否是智能指针的误报 - 更精确的检测"""
        if issue_type not in ['raw_ptr_dereference', 'null_dereference']:
            return False

        # 检查行中是否包含智能指针变量的->操作
        for var in smart_pointer_vars:
            if var + '->' in line:
                return True

        # 检查是否是std::shared_ptr或std::unique_ptr的直接使用
        if 'std::shared_ptr' in line or 'std::unique_ptr' in line:
            return True

        # 检查是否是智能指针get()方法的调用结果
        if '.get()' in line and '->' in line:
            return True

        # 检查是否有空指针检查模式 - 更全面的检查
        null_check_patterns = [
            r'if\s*\(\s*!?\s*[a-zA-Z_][a-zA-Z0-9_]*\s*\)',  # if (!ptr) 或 if (ptr)
            r'[a-zA-Z_][a-zA-Z0-9_]*\s*!=\s*nullptr',  # ptr != nullptr
            r'nullptr\s*!=\s*[a-zA-Z_][a-zA-Z0-9_]*',  # nullptr != ptr
            r'[a-zA-Z_][a-zA-Z0-9_]*\s*==\s*nullptr',  # ptr == nullptr
            r'nullptr\s*==\s*[a-zA-Z_][a-zA-Z0-9_]*',  # nullptr == ptr
            r'!\s*[a-zA-Z_][a-zA-Z0-9_]*',  # !ptr
            r'[a-zA-Z_][a-zA-Z0-9_]*\s*&&',  # ptr &&
            r'[a-zA-Z_][a-zA-Z0-9_]*\s*\|\|',  # ptr ||
            r'\?\s*[a-zA-Z_][a-zA-Z0-9_]*\s*:',  # 三元运算符 ptr ? :
        ]

        for pattern in null_check_patterns:
            if re.search(pattern, line):
                return True  # 如果有空指针检查，认为不是误报

        # 检查函数调用中是否已经有空指针检查（前几行）
        # 这里需要访问更多上下文，但暂时简化处理

        return False

    def _get_severity(self, issue_type, line):
        """获取问题严重性等级"""
        high_severity = ['raw_new_delete', 'double_delete', 'null_dereference', 
                        'uninitialized_pointer', 'missing_delete']
        medium_severity = ['raw_malloc_free', 'raw_ptr_dereference', 
                          'c_style_cast', 'void_pointer_arithmetic']
        
        if issue_type in high_severity:
            return 'HIGH'
        elif issue_type in medium_severity:
            return 'MEDIUM'
        else:
            return 'LOW'
    
    def scan_directory(self, base_path='.'):
        """扫描指定目录下的所有C++源文件"""
        cpp_extensions = ['.cpp', '.cc', '.h', '.hpp']
        
        for target_dir in self.target_dirs:
            dir_path = os.path.join(base_path, target_dir)
            if os.path.exists(dir_path):
                for root, dirs, files in os.walk(dir_path):
                    for file in files:
                        if any(file.endswith(ext) for ext in cpp_extensions):
                            filepath = os.path.join(root, file)
                            self.analyze_file(filepath)
    
    def generate_report(self):
        """生成分析报告"""
        report = []
        report.append("# SQLCC 内存安全审计报告")
        report.append(f"生成时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
        report.append("")
        
        # 统计信息
        total_issues = len(self.issues)
        high_issues = len([i for i in self.issues if i['severity'] == 'HIGH'])
        medium_issues = len([i for i in self.issues if i['severity'] == 'MEDIUM'])
        low_issues = len([i for i in self.issues if i['severity'] == 'LOW'])
        
        report.append("## 总体统计")
        report.append(f"- 总问题数: {total_issues}")
        report.append(f"- 高风险问题: {high_issues}")
        report.append(f"- 中风险问题: {medium_issues}")
        report.append(f"- 低风险问题: {low_issues}")
        report.append("")
        
        # 按文件分组
        report.append("## 问题详情")
        
        for filepath, issues in sorted(self.file_issues.items()):
            if not issues:
                continue
                
            report.append(f"### {filepath}")
            
            high_in_file = len([i for i in issues if i['severity'] == 'HIGH'])
            medium_in_file = len([i for i in issues if i['severity'] == 'MEDIUM'])
            low_in_file = len([i for i in issues if i['severity'] == 'LOW'])
            
            report.append(f"问题数: {len(issues)} (高:{high_in_file}, 中:{medium_in_file}, 低:{low_in_file})")
            report.append("")
            
            for issue in issues:
                severity_icon = {'HIGH': '🔴', 'MEDIUM': '🟡', 'LOW': '🟢'}[issue['severity']]
                report.append(f"- **{severity_icon} {issue['severity']}** (行{issue['line']}): {issue['type']}")
                report.append(f"  ```")
                report.append(f"  {issue['content'].strip()}")
                report.append(f"  匹配: {issue['match']}")
                report.append(f"  ```")
                report.append("")
        
        # 修复建议
        report.append("## 修复建议")
        report.append("### 高优先级修复")
        report.append("1. **替换裸指针使用**: 使用智能指针 (std::unique_ptr, std::shared_ptr)")
        report.append("2. **内存泄漏检查**: 确保每个new都有对应的delete")
        report.append("3. **空指针检查**: 在解引用前检查指针有效性")
        report.append("4. **RAII模式**: 使用RAII模式管理资源")
        report.append("")
        
        report.append("### 中优先级修复")
        report.append("1. **C风格类型转换**: 替换为C++类型转换操作符")
        report.append("2. **void指针运算**: 避免void指针算术运算")
        report.append("3. **缓冲区安全**: 使用安全的缓冲区操作函数")
        report.append("")
        
        report.append("### 代码示例")
        report.append("```cpp")
        report.append("// 不安全 - 裸指针")
        report.append("char* buffer = new char[100];")
        report.append("// 使用后忘记delete")
        report.append("")
        report.append("// 安全 - 智能指针")
        report.append("std::unique_ptr<char[]> buffer(new char[100]);")
        report.append("// 自动管理内存，无需手动delete")
        report.append("```")
        report.append("")
        
        return '\n'.join(report)
    
    def save_report(self, filename):
        """保存报告到文件"""
        report = self.generate_report()
        with open(filename, 'w', encoding='utf-8') as f:
            f.write(report)
        print(f"报告已保存到: {filename}")

def main():
    auditor = MemoryAuditor()
    
    print("开始内存安全审计...")
    print("扫描目录:", auditor.target_dirs)
    
    auditor.scan_directory()
    
    # 生成报告
    report_filename = f"内存安全审计报告_{datetime.now().strftime('%Y%m%d_%H%M%S')}.md"
    auditor.save_report(report_filename)
    
    # 显示简要统计
    total = len(auditor.issues)
    high = len([i for i in auditor.issues if i['severity'] == 'HIGH'])
    print(f"\n审计完成!")
    print(f"发现问题: {total}个")
    print(f"高风险问题: {high}个")
    print(f"报告文件: {report_filename}")
    
    if high > 0:
        print("\n⚠️  发现高风险内存安全问题，请优先处理!")
        return 1
    else:
        print("\n✅ 未发现高风险内存安全问题!")
        return 0

if __name__ == "__main__":
    sys.exit(main())
