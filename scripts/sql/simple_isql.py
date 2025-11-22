#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
简化版 isql 工具
用于演示如何读取和执行 SQL 脚本
"""

import sys
import os

def parse_sql_file(file_path):
    """解析SQL文件，按分号分割SQL语句"""
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
            
        # 去除注释
        lines = content.split('\n')
        clean_content = []
        in_comment = False
        
        for line in lines:
            # 处理行注释
            if '--' in line:
                line = line.split('--')[0]
            # 处理多行注释（简化版）
            if '/*' in line:
                in_comment = True
                if '*/' in line:
                    in_comment = False
                    continue
            if '*/' in line:
                in_comment = False
                continue
            if not in_comment and line.strip():
                clean_content.append(line)
        
        # 按分号分割SQL语句
        sql_content = '\n'.join(clean_content)
        sql_statements = []
        current_statement = ''
        
        for char in sql_content:
            current_statement += char
            if char == ';':
                sql_statements.append(current_statement.strip())
                current_statement = ''
        
        # 处理最后一个没有分号的语句（如果有）
        if current_statement.strip():
            sql_statements.append(current_statement.strip())
        
        return sql_statements
        
    except FileNotFoundError:
        print(f"错误: 找不到文件 '{file_path}'")
        return []
    except Exception as e:
        print(f"错误: 读取文件时出错 - {str(e)}")
        return []

def execute_sql_statement(statement):
    """模拟执行SQL语句"""
    statement = statement.strip()
    if not statement:
        return
    
    # 简化的语句类型识别
    stmt_upper = statement.upper()
    
    if stmt_upper.startswith('CREATE TABLE'):
        # 提取表名
        parts = statement.split()
        if len(parts) >= 3:
            table_name = parts[2]
            # 去除括号（如果有）
            if table_name.startswith('('):
                table_name = table_name[1:]
            print(f"[执行成功] 创建表: {table_name}")
        else:
            print(f"[执行] CREATE TABLE 语句")
            
    elif stmt_upper.startswith('INSERT INTO'):
        print(f"[执行成功] INSERT 语句: {statement[:50]}...")
        
    elif stmt_upper.startswith('SELECT'):
        print(f"[执行成功] SELECT 查询: {statement[:50]}...")
        print("[查询结果模拟显示]")
        print("+------------------+")
        print("| 模拟数据行       |")
        print("+------------------+")
        print("| 数据已成功查询   |")
        print("+------------------+")
        
    elif stmt_upper.startswith('UPDATE'):
        print(f"[执行成功] UPDATE 语句: {statement[:50]}...")
        
    elif stmt_upper.startswith('DELETE'):
        print(f"[执行成功] DELETE 语句: {statement[:50]}...")
        
    elif stmt_upper.startswith('ALTER TABLE'):
        print(f"[执行成功] ALTER TABLE 语句: {statement[:50]}...")
        
    elif stmt_upper.startswith('DROP TABLE'):
        print(f"[执行成功] DROP TABLE 语句: {statement[:50]}...")
        
    else:
        print(f"[执行] SQL语句: {statement[:50]}...")

def show_help():
    """显示帮助信息"""
    help_text = """
简化版 isql 工具

用法:
  python simple_isql.py -f <sql_file>    执行指定的SQL脚本文件
  python simple_isql.py -h               显示此帮助信息
  python simple_isql.py -v               显示版本信息

示例:
  python simple_isql.py -f scripts/sql/comprehensive_test.sql
    """
    print(help_text)

def show_version():
    """显示版本信息"""
    print("简化版 isql 工具 v1.0")
    print("用于演示 SQL 脚本执行过程")

def main():
    """主函数"""
    if len(sys.argv) < 2:
        show_help()
        return
    
    if sys.argv[1] == '-h':
        show_help()
    elif sys.argv[1] == '-v':
        show_version()
    elif sys.argv[1] == '-f' and len(sys.argv) > 2:
        sql_file = sys.argv[2]
        print(f"正在执行 SQL 脚本: {sql_file}")
        print("=" * 60)
        
        sql_statements = parse_sql_file(sql_file)
        if sql_statements:
            print(f"找到 {len(sql_statements)} 条 SQL 语句")
            print()
            
            for i, statement in enumerate(sql_statements, 1):
                print(f"语句 {i}: {statement[:80]}...")
                execute_sql_statement(statement)
                print()
            
            print("=" * 60)
            print(f"SQL 脚本执行完成！共执行 {len(sql_statements)} 条语句")
    else:
        show_help()

if __name__ == "__main__":
    main()