#!/bin/bash

# 测试不支持的DCL和DDL命令

# 创建测试目录
mkdir -p test_output

# 测试命令列表
unsupported_commands=(
    "ALTER USER test_user IDENTIFIED BY new_password;"
    "DROP USER test_user;"
    "CREATE ROLE admin;"
    "DROP ROLE admin;"
    "ALTER ROLE admin SET password = 'new_pass';"
    "SET ROLE admin;"
    "CREATE VIEW v1 AS SELECT * FROM users;"
    "DROP VIEW v1;"
    "ALTER VIEW v1 AS SELECT id FROM users;"
    "CREATE SCHEMA test_schema;"
    "DROP SCHEMA test_schema;"
    "ALTER SCHEMA test_schema RENAME TO new_schema;"
    "TRUNCATE TABLE users;"
    "RENAME TABLE users TO new_users;"
)

# 支持的命令列表（用于对比）
supported_commands=(
    "CREATE USER test_user IDENTIFIED BY password;"
    "GRANT SELECT ON users TO test_user;"
    "REVOKE SELECT ON users FROM test_user;"
    "CREATE TABLE test_table (id INT, name VARCHAR(50));"
    "DROP TABLE test_table;"
    "CREATE INDEX idx_test ON test_table(id);"
)

# 测试函数
test_command() {
    local command="$1"
    local expected_output="$2"
    local output
    
    echo "测试命令: $command"
    output=$(./test_working_dir/build/bin/sqlcc -e "$command" 2>&1 || true)
    
    if [[ "$output" == *"$expected_output"* ]]; then
        echo -e "✅ 预期结果: $expected_output"
        echo -e "   实际结果: $output"
        return 0
    else
        echo -e "❌ 预期结果: $expected_output"
        echo -e "   实际结果: $output"
        return 1
    fi
    echo ""
}

# 运行测试
echo "====================================="
echo "SQLCC 不支持的命令测试"
echo "====================================="
echo ""

# 测试不支持的命令
echo "测试不支持的命令:"
echo "-------------------------------------"
failed=0
total=0

for cmd in "${unsupported_commands[@]}"; do
    total=$((total+1))
    if test_command "$cmd" "Command not supported"; then
        : # 测试通过
    else
        failed=$((failed+1))
    fi
echo ""
done

# 测试支持的命令
echo "测试支持的命令:"
echo "-------------------------------------"

for cmd in "${supported_commands[@]}"; do
    total=$((total+1))
    if test_command "$cmd" "Command not supported"; then
        failed=$((failed+1))
    else
        echo -e "✅ 命令支持: $cmd"
    fi
echo ""
done

# 输出结果
echo "====================================="
echo "测试结果:"
echo "总测试数: $total"
echo "通过数: $((total-failed))"
echo "失败数: $failed"

if [ $failed -eq 0 ]; then
    echo -e "✅ 所有测试通过!"
    exit 0
else
    echo -e "❌ 有 $failed 个测试失败"
    exit 1
fi