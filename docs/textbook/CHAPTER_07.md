# 《数据库系统原理与开发实践》 - 第7章：编译原理在SQL处理中的实践应用

**词法分析、语法解析与查询优化的编译器技术**

---

## 🎯 **本章核心目标**

理解编译原理核心技术如何驱动SQL查询处理的完整流程：
- SQL词法分析与语法解析的技术实现
- 查询优化算法的编译器理论支撑
- AST抽象语法树与代码生成的设计模式

---

## 7.1 SQL词法分析与正则表达式技术

### 7.1.1 词素识别与token化过程

SQL编译的第一步是词法分析，将原始字符串分解为有意义的token序列：

```cpp
class SQLLexer {
private:
    std::string input;
    size_t position;

    // Token定义
    enum TokenType {
        SELECT, FROM, WHERE, INSERT, UPDATE, DELETE,
        IDENTIFIER, NUMBER, STRING,
        OPERATOR_EQ, OPERATOR_LT, OPERATOR_GT,
        COMMA, SEMICOLON, LPAREN, RPAREN,
        KEYWORD_AND, KEYWORD_OR
    };

    struct Token {
        TokenType type;
        std::string value;
        size_t line, column;  // 错误定位
    };

public:
    std::vector<Token> tokenize(const std::string& sql) {
        input = sql;
        position = 0;
        std::vector<Token> tokens;

        while (position < input.length()) {
            skip_whitespace();

            if (position >= input.length()) break;

            char ch = input[position];

            if (is_identifier_start(ch)) {
                tokens.push_back(tokenize_identifier());
            }
            else if (is_digit(ch)) {
                tokens.push_back(tokenize_number());
            }
            else if (ch == '"') {
                tokens.push_back(tokenize_string());
            }
            else if (is_operator(ch)) {
                tokens.push_back(tokenize_operator());
            }
            else if (is_punctuation(ch)) {
                tokens.push_back(tokenize_punctuation());
            }
            else {
                error("Unexpected character", position);
            }
        }

        tokens.push_back({EOF_TOKEN, "", 0, 0});
        return tokens;
    }

private:
    void skip_whitespace() {
        while (position < input.length() &&
               std::isspace(input[position])) {
            position++;
        }
    }

    Token tokenize_identifier() {
        size_t start = position;

        while (position < input.length() &&
               is_identifier_char(input[position])) {
            position++;
        }

        std::string lexeme = input.substr(start, position - start);

        // 检查是否为关键字
        TokenType type = get_keyword_type(lexeme);
        if (type == IDENTIFIER) {
            type = IDENTIFIER;
        }

        return {type, lexeme, 0, start}; // 简化行号
    }
};
```

### 7.1.2 词法状态机设计

使用有限状态自动机(Finite State Machine)处理复杂词素：

```cpp
class LexerFSM {
private:
    enum State {
        START,
        IN_IDENTIFIER,
        IN_NUMBER,
        IN_STRING,
        IN_COMMENT,
        DONE
    };

    State current_state;

    void advance_state(char ch) {
        switch (current_state) {
            case START:
                if (is_letter(ch)) {
                    current_state = IN_IDENTIFIER;
                    add_char(ch);
                }
                else if (is_digit(ch)) {
                    current_state = IN_NUMBER;
                    add_char(ch);
                }
                else if (ch == '"') {
                    current_state = IN_STRING;
                }
                else if (ch == '-') {
                    if (peek() == '-') {
                        current_state = IN_COMMENT;
                        advance(); // consume second -
                    }
                    else {
                        // single - operator
                        current_state = DONE;
                        token.type = OPERATOR_MINUS;
                    }
                }
                // 处理其他情况...
                break;

            case IN_STRING:
                if (ch == '"') {
                    current_state = DONE;
                    token.type = STRING_LITERAL;
                }
                else if (ch == '\\') {
                    // 处理转义字符
                    add_char(unescape_next());
                }
                else {
                    add_char(ch);
                }
                break;

            // 其他状态...
        }
    }

public:
    Token next_token(const std::string& input) {
        current_state = START;
        token.clear();

        for (char ch : input) {
            advance_state(ch);

            if (current_state == DONE) {
                return token;
            }
        }

        return token; // EOF
    }
};
```

## 7.2 SQL语法解析的递归下降方法

### 7.2.1 LL(1)语法解析算法

SQL语法解析使用递归下降算法，配合预测分析表：

```cpp
class SQLParser {
private:
    std::vector<Token>::iterator current_token;
    std::vector<Token> tokens;

public:
    ASTNode* parse(const std::vector<Token>& token_list) {
        tokens = token_list;
        current_token = tokens.begin();

        try {
            return parse_query();
        } catch (const std::runtime_error& e) {
            std::cout << "Parse error: " << e.what() << std::endl;
            return nullptr;
        }
    }

private:
    // SELECT-FROM-WHERE 基本结构
    SelectStmt* parse_query() {
        consume(SELECT);  // 消耗SELECT关键字

        std::vector<Expression*> select_list = parse_select_list();

        consume(FROM);
        TableRef* table = parse_table_reference();

        WhereClause* where = nullptr;
        if (current_token->type == WHERE) {
            consume(WHERE);
            where = new WhereClause(parse_expression());
        }

        return new SelectStmt(select_list, table, where);
    }

    std::vector<Expression*> parse_select_list() {
        std::vector<Expression*> items;

        do {
            if (current_token->type == ASTERISK) {
                consume(ASTERISK);
                items.push_back(new AllColumnsExpr());
            } else {
                items.push_back(parse_expression());
            }
        } while (match(COMMA));

        return items;
    }

    Expression* parse_expression() {
        return parse_or_expression();
    }

    Expression* parse_or_expression() {
        Expression* left = parse_and_expression();

        while (match(OR)) {
            Token op = previous();
            Expression* right = parse_and_expression();
            left = new BinaryExpr(left, op, right);
        }

        return left;
    }

    // 其他表达式解析...
};
```

### 7.2.2 AST构建与语义分析

构建抽象语法树并进行语义检查：

```cpp
class ASTBuilder {
private:
    SymbolTable symbol_table;

public:
    ASTNode* build_ast(const std::vector<Token>& tokens) {
        SQLParser parser;
        ASTNode* raw_ast = parser.parse(tokens);

        if (raw_ast) {
            perform_semantic_analysis(raw_ast);
            resolve_identifiers(raw_ast);
        }

        return raw_ast;
    }

private:
    void perform_semantic_analysis(ASTNode* node) {
        if (SelectStmt* select = dynamic_cast<SelectStmt*>(node)) {
            check_select_semantics(select);
        }
        // 递归处理子节点
        for (ASTNode* child : node->children) {
            perform_semantic_analysis(child);
        }
    }

    void check_select_semantics(SelectStmt* select) {
        // 检查表是否存在
        for (TableRef* table : select->tables) {
            if (!symbol_table.table_exists(table->name)) {
                error("Table '" + table->name + "' does not exist");
            }
        }

        // 检查列是否存在
        for (Expression* expr : select->select_list) {
            if (ColumnRef* col = dynamic_cast<ColumnRef*>(expr)) {
                if (!symbol_table.column_exists(col->table, col->column)) {
                    error("Column '" + col->column + "' does not exist");
                }
            }
        }

        // 类型兼容性检查
        check_type_compatibility(select->where_clause);
    }
};
```

## 7.3 查询优化的代价模型与重写技术

### 7.3.1 基于代价的查询优化框架

查询优化器使用代价模型选择最优执行计划：

```cpp
class QueryOptimizer {
private:
    CostModel* cost_model;
    StatisticsCollector* stats;

public:
    QueryPlan* optimize(SelectStmt* query, Schema* schema) {
        // 1. 生成候选执行计划
        std::vector<QueryPlan*> candidates = generate_plans(query, schema);

        // 2. 为每个计划估算代价
        for (QueryPlan* plan : candidates) {
            plan->cost = estimate_cost(plan);
        }

        // 3. 选择代价最小的计划
        return select_cheapest_plan(candidates);
    }

private:
    double estimate_cost(QueryPlan* plan) {
        double cost = 0.0;

        if (TableScan* scan = dynamic_cast<TableScan*>(plan)) {
            cost += estimate_table_scan_cost(scan);
        }
        else if (IndexScan* index_scan = dynamic_cast<IndexScan*>(plan)) {
            cost += estimate_index_scan_cost(index_scan);
        }
        else if (NestedLoopJoin* join = dynamic_cast<NestedLoopJoin*>(plan)) {
            cost += estimate_join_cost(join);
        }
        // 加上CPU和I/O代价

        return cost;
    }

    double estimate_table_scan_cost(TableScan* scan) {
        TableStats* table_stats = stats->get_table_stats(scan->table_name);

        // 假设顺序扫描
        double io_cost = table_stats->num_pages;  // I/O代价
        double cpu_cost = table_stats->num_tuples * 0.01; // CPU代价

        // 应用选择率
        if (scan->predicate) {
            double selectivity = estimate_selectivity(scan->predicate);
            io_cost *= selectivity;
            cpu_cost *= selectivity;
        }

        return io_cost + cpu_cost;
    }
};
```

### 7.3.2 查询重写技术的等价变换

使用代数等价变换优化查询：

```cpp
class QueryRewriter {
public:
    SelectStmt* rewrite(SelectStmt* query) {
        SelectStmt* rewritten = query->clone();

        // 应用一系列重写规则
        rewritten = apply_projection_pushdown(rewritten);
        rewritten = apply_selection_pushdown(rewritten);
        rewritten = eliminate_redundant_joins(rewritten);
        rewritten = merge_identical_selects(rewritten);

        return rewritten;
    }

private:
    SelectStmt* apply_selection_pushdown(SelectStmt* query) {
        // 将WHERE条件下推到子查询或连接
        // σ(A∧B)(R⋈S) → σ(A)(σ(B)(R⋈S)) 如果B只涉及R

        for (Expression* condition : query->where_clause->conditions) {
            // 检查condition是否只涉及单个表
            if (can_push_down(condition)) {
                // 将条件移到对应的表扫描
                push_condition_to_table(condition);
                remove_from_where_clause(condition);
            }
        }

        return query;
    }

    SelectStmt* apply_projection_pushdown(SelectStmt* query) {
        // 只保留执行过程中需要的列
        // π(L)(σ(C)(R)) → σ(C)(π(L)(R))

        std::set<std::string> required_columns = collect_required_columns(query);

        for (TableRef* table : query->from_clause->tables) {
            if (table->scan_type == TABLE_SCAN) {
                // 只扫描需要的列
                restrict_projection(table->scan_node, required_columns);
            }
        }

        return query;
    }
};
```

### 7.3.3 动态规划与连接顺序优化

处理多表连接时的最优连接顺序选择：

```cpp
class JoinOrderOptimizer {
private:
    CostModel* cost_model;

public:
    JoinPlan* find_optimal_join_order(std::vector<TableRef*>& tables) {
        int n = tables.size();
        std::vector<std::vector<JoinPlan*>> dp(n+1,
            std::vector<JoinPlan*>(1 << n, nullptr));

        // 初始化单个表的访问代价
        for (int i = 0; i < n; i++) {
            dp[1][1 << i] = create_base_plan(tables[i]);
        }

        // 动态规划填充
        for (int size = 2; size <= n; size++) {
            for (int mask = 0; mask < (1 << n); mask++) {
                if (__builtin_popcount(mask) != size) continue;

                dp[size][mask] = find_best_plan_for_subset(tables, mask, size);
            }
        }

        return dp[n][(1 << n) - 1];  // 完整集合的最优计划
    }

private:
    JoinPlan* find_best_plan_for_subset(std::vector<TableRef*>& tables,
                                      int mask, int size) {
        JoinPlan* best_plan = nullptr;
        double min_cost = std::numeric_limits<double>::max();

        // 尝试分割子集的所有方式
        for (int left_size = 1; left_size < size; left_size++) {
            int right_size = size - left_size;

            for (int left_mask : get_subsets_of_size(mask, left_size)) {
                int right_mask = mask ^ left_mask;  // 异或得到右子集

                JoinPlan* left_plan = dp[left_size][left_mask];
                JoinPlan* right_plan = dp[right_size][right_mask];

                if (!left_plan || !right_plan) continue;

                // 生成所有可能的连接算法
                for (JoinMethod method : {NESTED_LOOP, HASH_JOIN, MERGE_JOIN}) {
                    JoinPlan* combined = create_join_plan(
                        left_plan, right_plan, method);
                    double cost = cost_model->estimate_join_cost(combined);

                    if (cost < min_cost) {
                        min_cost = cost;
                        best_plan = combined;
                    }
                }
            }
        }

        return best_plan;
    }
};
```

## 7.4 查询执行计划的代码生成技术

### 7.4.1 迭代器模式的执行计划实现

```cpp
class QueryIterator {
public:
    virtual bool open() = 0;
    virtual Tuple* next() = 0;
    virtual void close() = 0;
    virtual ~QueryIterator() {}
};

// 表扫描迭代器
class TableScanIterator : public QueryIterator {
private:
    Table* table;
    size_t current_page;
    size_t current_tuple_in_page;

public:
    bool open() override {
        current_page = 0;
        current_tuple_in_page = 0;
        return true;
    }

    Tuple* next() override {
        while (current_page < table->num_pages) {
            Page* page = table->get_page(current_page);
            Tuple* tuple = page->get_tuple(current_tuple_in_page);

            if (tuple) {
                current_tuple_in_page++;
                return tuple;
            }

            // 当前页面处理完毕，移到下一页
            current_page++;
            current_tuple_in_page = 0;
        }

        return nullptr;  // 没有更多元组
    }

    void close() override {
        // 清理资源
    }
};

// WHERE条件过滤迭代器
class FilterIterator : public QueryIterator {
private:
    QueryIterator* input;
    Expression* predicate;

public:
    FilterIterator(QueryIterator* input, Expression* pred)
        : input(input), predicate(pred) {}

    bool open() override {
        return input->open();
    }

    Tuple* next() override {
        while (Tuple* tuple = input->next()) {
            if (evaluate_predicate(predicate, tuple)) {
                return tuple;
            }
            // 不满足条件，获取下一个
        }
        return nullptr;
    }

    void close() override {
        input->close();
    }

    bool evaluate_predicate(Expression* expr, Tuple* tuple) {
        // 评估表达式是否为真
        return true; // 简化实现
    }
};
```

## 📚 **本章总结：编译原理在数据库中的完美体现**

编译原理为SQL查询处理提供了完整的理论框架，从词法语法分析到查询优化，每一步都体现了编译器技术的优雅应用。

**核心洞察**：
- **词法语法分析**: 正则表达式与有限状态机将SQL文本转换为结构化表示
- **语义分析**: 符号表与类型检查保证查询的正确性和安全性
- **查询优化**: 代价模型与重写规则实现自动性能优化
- **代码生成**: 迭代器模式将逻辑计划转换为高效执行代码

编译原理不仅解释了数据库如何处理SQL查询，更为我们展示了理论与实践完美结合的艺术。

---

**思考题**：
1. SQL词法分析在设计时需要考虑哪些特殊情况？
2. 为什么查询优化是DBMS性能的关键？代价估计的准确性为何重要？
3. 编译原理的哪些技术在数据库查询处理中得到了应用？
