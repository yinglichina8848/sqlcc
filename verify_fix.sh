#!/bin/bash

echo "=== 验证SQL解析器修复效果 ==="
echo

# 检查源文件中的修改
echo "1. 检查parseUpdateStatement()函数:"
cd /home/liying/sqlcc
grep -n -A 3 "parseUpdateStatement()" src/sql_parser/parser_new.cpp | head -5

echo
echo "2. 检查parseDeleteStatement()函数中是否删除了重复的consume调用:"
grep -n -A 5 -B 2 "DELETE关键字已经被消费" src/sql_parser/parser_new.cpp

echo
echo "3. 确认没有剩余的重复consume调用:"
if grep -q "consume(Token::KEYWORD_UPDATE)" src/sql_parser/parser_new.cpp; then
    echo "❌ 警告：发现剩余的UPDATE关键字消费调用"
else
    echo "✅ UPDATE解析函数已正确修改"
fi

if grep -q "consume(Token::KEYWORD_DELETE)" src/sql_parser/parser_new.cpp; then
    echo "❌ 警告：发现剩余的DELETE关键字消费调用"
else
    echo "✅ DELETE解析函数已正确修改"
fi

echo
echo "=== 修复总结 ==="
echo "问题原因：parseDMLStatement()中的match()方法已经消费了关键字，但各自的解析函数又重复消费"
echo "解决方案：删除parseUpdateStatement()和parseDeleteStatement()中的重复consume()调用"
echo "预期结果：UPDATE和DELETE语句解析应该成功，语句计数不再为0"