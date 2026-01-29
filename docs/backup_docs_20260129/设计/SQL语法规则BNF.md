# SQLCC SQL 语法规则 (BNF/EBNF)

本文档定义了SQLCC解析器支持的SQL语法规则，使用扩展巴克斯范式（EBNF）表示法。

## 语法约定

- `::=` 表示定义
- `|` 表示选择
- `[]` 表示可选
- `{}` 表示零次或多次重复
- `()` 表示分组
- `""` 或 `''` 表示终端符号（字面值）
- `*` 表示零次或多次重复
- `+` 表示一次或多次重复
- `?` 表示零次或一次（可选）

## 1. 词法规则 (Lexical Rules)

### 1.1 标识符 (Identifiers)
```
identifier ::= letter { letter | digit | '_' }
letter ::= 'a'..'z' | 'A'..'Z'
digit ::= '0'..'9'
```

### 1.2 字面量 (Literals)
```
string_literal ::= "'" { character | escaped_char } "'"
                  | '"' { character | escaped_char } '"'

integer_literal ::= digit+

float_literal ::= digit+ '.' digit+
                  | '.' digit+

boolean_literal ::= 'TRUE' | 'FALSE'

null_literal ::= 'NULL'
```

### 1.3 注释 (Comments)
```
single_line_comment ::= '--' { any_character_except_newline }

multi_line_comment ::= '/*' { any_character } '*/'
```

### 1.4 操作符 (Operators)
```
comparison_operator ::= '=' | '!=' | '<>' | '<' | '<=' | '>' | '>='

arithmetic_operator ::= '+' | '-' | '*' | '/'

logical_operator ::= 'AND' | 'OR' | 'NOT'

assignment_operator ::= '='

concat_operator ::= '||'
```

## 2. 语法规则 (Syntax Rules)

### 2.1 程序结构 (Program Structure)
```
sql_program ::= { statement ';' } [ statement ]

statement ::= ddl_statement
            | dml_statement
            | dcl_statement
            | tcl_statement
            | show_statement
```

### 2.2 DDL 语句 (Data Definition Language)

#### 2.2.1 CREATE TABLE
```
create_table_statement ::= 'CREATE' 'TABLE' [ 'IF' 'NOT' 'EXISTS' ] table_name
                          '(' column_definition { ',' column_definition }* ')'

column_definition ::= column_name data_type [ column_constraints ]

column_constraints ::= { column_constraint }*

column_constraint ::= 'PRIMARY' 'KEY'
                     | 'NOT' 'NULL'
                     | 'NULL'
                     | 'UNIQUE'
                     | 'AUTO_INCREMENT'
                     | 'DEFAULT' ( literal | 'NULL' )
                     | 'REFERENCES' table_name '(' column_name ')'
                     | 'CHECK' '(' expression ')'

data_type ::= 'INT' [ '(' integer_literal ')' ]
             | 'VARCHAR' '(' integer_literal ')'
             | 'TEXT'
             | 'DATE'
             | 'DATETIME'
             | 'DECIMAL' '(' integer_literal ',' integer_literal ')'
             | 'FLOAT'
             | 'DOUBLE'
             | 'BOOLEAN'
             | 'BLOB'
```

#### 2.2.2 CREATE INDEX
```
create_index_statement ::= 'CREATE' [ 'UNIQUE' ] 'INDEX' index_name
                          'ON' table_name '(' column_name { ',' column_name }* ')'
```

#### 2.2.3 DROP TABLE/INDEX
```
drop_table_statement ::= 'DROP' 'TABLE' [ 'IF' 'EXISTS' ] table_name

drop_index_statement ::= 'DROP' 'INDEX' [ 'IF' 'EXISTS' ] index_name
                         [ 'ON' table_name ]
```

#### 2.2.4 ALTER TABLE
```
alter_table_statement ::= 'ALTER' 'TABLE' table_name alter_action

alter_action ::= 'ADD' 'COLUMN' column_definition
                | 'DROP' 'COLUMN' column_name
                | 'MODIFY' 'COLUMN' column_definition
                | 'ADD' table_constraint
                | 'DROP' 'CONSTRAINT' constraint_name
```

### 2.3 DML 语句 (Data Manipulation Language)

#### 2.3.1 SELECT 语句
```
select_statement ::= 'SELECT' [ 'DISTINCT' | 'ALL' ] select_list
                     [ 'FROM' from_clause ]
                     [ 'WHERE' where_clause ]
                     [ 'GROUP' 'BY' group_by_clause ]
                     [ 'HAVING' having_clause ]
                     [ 'ORDER' 'BY' order_by_clause ]
                     [ 'LIMIT' limit_clause ]
                     [ 'OFFSET' offset_clause ]
                     [ compound_operator select_statement ]*

select_list ::= '*' | select_item { ',' select_item }*

select_item ::= expression [ [ 'AS' ] alias ]

from_clause ::= table_reference { join_clause }*

table_reference ::= table_name [ [ 'AS' ] alias ]
                   | '(' select_statement ')' [ [ 'AS' ] alias ]
                   | table_reference join_clause

join_clause ::= [ join_type ] 'JOIN' table_reference [ 'ON' join_condition ]

join_type ::= 'INNER' | 'LEFT' [ 'OUTER' ] | 'RIGHT' [ 'OUTER' ] | 'FULL' [ 'OUTER' ]

join_condition ::= expression

where_clause ::= expression

group_by_clause ::= expression { ',' expression }*

having_clause ::= expression

order_by_clause ::= order_by_item { ',' order_by_item }*

order_by_item ::= expression [ 'ASC' | 'DESC' ]

limit_clause ::= integer_literal

offset_clause ::= integer_literal

compound_operator ::= 'UNION' [ 'ALL' ] | 'INTERSECT' | 'EXCEPT'
```

#### 2.3.2 INSERT 语句
```
insert_statement ::= 'INSERT' 'INTO' table_name
                     [ '(' column_name { ',' column_name }* ')' ]
                     ( 'VALUES' '(' value_list { ',' value_list }* ')'
                     | select_statement )

value_list ::= expression { ',' expression }*
```

#### 2.3.3 UPDATE 语句
```
update_statement ::= 'UPDATE' table_name
                     'SET' assignment { ',' assignment }*
                     [ 'WHERE' where_clause ]

assignment ::= column_name '=' expression
```

#### 2.3.4 DELETE 语句
```
delete_statement ::= 'DELETE' 'FROM' table_name
                     [ 'WHERE' where_clause ]
```

### 2.4 DCL 语句 (Data Control Language)

#### 2.4.1 GRANT 语句
```
grant_statement ::= 'GRANT' privilege_list
                    'ON' [ object_type ] object_name
                    'TO' user_name

privilege_list ::= privilege { ',' privilege }*

privilege ::= 'SELECT' | 'INSERT' | 'UPDATE' | 'DELETE' | 'ALL' [ 'PRIVILEGES' ]

object_type ::= 'TABLE' | 'DATABASE' | 'VIEW'
```

#### 2.4.2 REVOKE 语句
```
revoke_statement ::= 'REVOKE' privilege_list
                     'ON' [ object_type ] object_name
                     'FROM' user_name
```

### 2.5 TCL 语句 (Transaction Control Language)
```
commit_statement ::= 'COMMIT'

rollback_statement ::= 'ROLLBACK'
```

### 2.6 SHOW 语句
```
show_statement ::= 'SHOW' 'DATABASES'
                  | 'SHOW' 'TABLES' [ 'FROM' database_name ]
                  | 'SHOW' 'COLUMNS' 'FROM' table_name
                  | 'SHOW' 'INDEXES' 'FROM' table_name
                  | 'SHOW' 'CREATE' 'TABLE' table_name
                  | 'SHOW' 'GRANTS' 'FOR' user_name
```

### 2.7 表达式 (Expressions)

#### 2.7.1 表达式层次
```
expression ::= or_expression

or_expression ::= and_expression { 'OR' and_expression }*

and_expression ::= not_expression { 'AND' not_expression }*

not_expression ::= [ 'NOT' ] comparison_expression

comparison_expression ::= additive_expression [ comparison_operator additive_expression ]

additive_expression ::= multiplicative_expression { ( '+' | '-' ) multiplicative_expression }*

multiplicative_expression ::= unary_expression { ( '*' | '/' | '%' ) unary_expression }*

unary_expression ::= [ ( '+' | '-' ) ] primary_expression

primary_expression ::= literal
                      | column_reference
                      | function_call
                      | '(' expression ')'
                      | 'CASE' when_clauses [ 'ELSE' expression ] 'END'
                      | 'EXISTS' '(' select_statement ')'
                      | subquery

column_reference ::= [ table_name '.' ] column_name

function_call ::= function_name '(' [ argument_list ] ')'

argument_list ::= expression { ',' expression }*

when_clauses ::= when_clause { when_clause }*

when_clause ::= 'WHEN' expression 'THEN' expression

subquery ::= '(' select_statement ')'
```

#### 2.7.2 函数 (Functions)
```
function_name ::= 'COUNT' | 'SUM' | 'AVG' | 'MIN' | 'MAX' | 'CONCAT'
                 | 'SUBSTRING' | 'LENGTH' | 'UPPER' | 'LOWER' | 'TRIM'
                 | user_defined_function_name
```

### 2.8 标识符和名称 (Identifiers and Names)
```
table_name ::= identifier

column_name ::= identifier

index_name ::= identifier

database_name ::= identifier

user_name ::= identifier

alias ::= identifier

constraint_name ::= identifier

object_name ::= identifier
```

## 3. 优先级和结合性 (Precedence and Associativity)

### 3.1 操作符优先级 (从高到低)
1. `.` (成员访问) - 左结合
2. `()` (函数调用) - 左结合
3. `+`, `-` (一元) - 右结合
4. `*`, `/`, `%` - 左结合
5. `+`, `-` (二元) - 左结合
6. `||` (字符串连接) - 左结合
7. 比较操作符 (`=`, `!=`, `<>`, `<`, `<=`, `>`, `>=`) - 无结合
8. `NOT` - 右结合
9. `AND` - 左结合
10. `OR` - 左结合

### 3.2 关键字大小写
所有SQL关键字不区分大小写，但标识符大小写敏感。

## 4. 扩展特性

### 4.1 子查询支持
- 标量子查询：`(SELECT ...)`
- 表子查询：在FROM子句中
- 存在性测试：`EXISTS (...)`
- 相关子查询：引用外部查询的列

### 4.2 JOIN 支持
- INNER JOIN
- LEFT [OUTER] JOIN
- RIGHT [OUTER] JOIN
- FULL [OUTER] JOIN
- 多表JOIN
- JOIN条件表达式

### 4.3 集合操作
- UNION [ALL]
- INTERSECT
- EXCEPT
- 集合操作的优先级和结合性

### 4.4 聚合函数
- COUNT(*), COUNT(column), COUNT(DISTINCT column)
- SUM, AVG, MIN, MAX
- GROUP BY 和 HAVING 子句

### 4.5 约束支持
- PRIMARY KEY
- FOREIGN KEY REFERENCES
- UNIQUE
- NOT NULL
- CHECK 约束
- DEFAULT 值

## 5. 错误处理

### 5.1 词法错误
- 无效字符
- 未终止的字符串
- 无效数字格式

### 5.2 语法错误
- 意外的token
- 缺少必需的token
- 无效的语法结构

### 5.3 语义错误
- 未定义的表/列
- 类型不匹配
- 约束违反

## 6. 实现注意事项

### 6.1 递归下降解析器设计
- 每个非终结符对应一个解析函数
- 使用前瞻token进行决策
- 错误恢复机制

### 6.2 AST 构建
- 每个语法规则构建相应的AST节点
- 保留位置信息用于错误报告
- 支持访问者模式

### 6.3 错误恢复策略
- Panic Mode Recovery
- 跳到下一个语句边界
- 收集多个错误信息

---

*本文档基于SQL标准定义，SQLCC解析器将支持上述语法规则的核心子集。*
