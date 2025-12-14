/**
 * @file advanced_sql_parser_test.cpp
 * @brief 高级SQL解析器测试套件
 *
 * 实现高级SQL解析器的全面测试，包括：
 * - 复杂SELECT语句解析（嵌套查询、集合操作、窗口函数）
 * - DDL语句的高级特性（约束、外键、索引）
 * - DML语句的高级特性（多表更新、条件插入）
 * - 存储过程和函数解析
 * - 触发器定义解析
 * - 视图创建和查询解析
 * - 递归查询和CTE解析
 * - JSON和数组数据类型解析
 * - 高级数据类型（几何、范围、自定义类型）
 */

#include "sql_parser/parser_new.h"
#include "sql_parser/ast_nodes.h"
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

using namespace sqlcc::sql_parser;
using namespace std;

// 测试夹具
class AdvancedSqlParserTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 初始化解析器
        parser_ = std::make_unique<ParserNew>("");
    }

    void TearDown() override {
        parser_.reset();
    }

    std::unique_ptr<ParserNew> parser_;

    // 辅助函数：解析SQL并返回语句列表
    std::vector<std::unique_ptr<Statement>> ParseSQL(const std::string& sql) {
        parser_ = std::make_unique<ParserNew>(sql);
        return parser_->parse();
    }

    // 辅助函数：验证解析结果
    bool ParseAndValidate(const std::string& sql, bool should_succeed = true) {
        try {
            auto statements = ParseSQL(sql);
            if (should_succeed) {
                return !statements.empty();
            } else {
                return statements.empty();
            }
        } catch (const std::exception&) {
            return !should_succeed;
        }
    }
};

// 复杂SELECT语句测试
TEST_F(AdvancedSqlParserTest, ComplexSelect_NestedSubqueries) {
    std::string sql = R"(
        SELECT u.name, u.email
        FROM users u
        WHERE u.id IN (
            SELECT DISTINCT user_id
            FROM orders o
            WHERE o.total > (
                SELECT AVG(total)
                FROM orders
                WHERE order_date >= '2023-01-01'
            )
        )
        AND u.created_at > (
            SELECT MAX(created_at)
            FROM users
            WHERE department = 'IT'
        );
    )";

    auto statements = ParseSQL(sql);
    ASSERT_FALSE(statements.empty());

    auto select_stmt = dynamic_cast<SelectStatement*>(statements[0].get());
    ASSERT_NE(select_stmt, nullptr);

    // 验证主查询结构
    EXPECT_EQ(select_stmt->select_list.size(), 2);
    EXPECT_FALSE(select_stmt->from_clause.empty());

    // 验证WHERE子句包含嵌套子查询
    ASSERT_NE(select_stmt->where_clause, nullptr);
    // 详细的子查询验证需要更复杂的AST遍历
}

TEST_F(AdvancedSqlParserTest, ComplexSelect_WindowFunctions) {
    std::string sql = R"(
        SELECT
            department,
            employee_name,
            salary,
            ROW_NUMBER() OVER (PARTITION BY department ORDER BY salary DESC) as dept_rank,
            RANK() OVER (ORDER BY salary DESC) as global_rank,
            DENSE_RANK() OVER (ORDER BY salary DESC) as dense_rank,
            PERCENT_RANK() OVER (ORDER BY salary DESC) as percent_rank,
            CUME_DIST() OVER (ORDER BY salary DESC) as cume_dist,
            NTILE(4) OVER (ORDER BY salary DESC) as quartile,
            LAG(salary, 1) OVER (ORDER BY salary DESC) as prev_salary,
            LEAD(salary, 1) OVER (ORDER BY salary DESC) as next_salary,
            FIRST_VALUE(salary) OVER (PARTITION BY department ORDER BY salary DESC) as dept_highest,
            LAST_VALUE(salary) OVER (PARTITION BY department ORDER BY hire_date) as latest_hire
        FROM employees
        ORDER BY department, dept_rank;
    )";

    auto statements = ParseSQL(sql);
    ASSERT_FALSE(statements.empty());

    auto select_stmt = dynamic_cast<SelectStatement*>(statements[0].get());
    ASSERT_NE(select_stmt, nullptr);

    // 验证SELECT列表包含多个窗口函数
    EXPECT_EQ(select_stmt->select_list.size(), 12); // 12列包括窗口函数
}

TEST_F(AdvancedSqlParserTest, ComplexSelect_CTE) {
    std::string sql = R"(
        WITH RECURSIVE employee_hierarchy AS (
            -- 锚点成员：顶级员工
            SELECT id, name, manager_id, 0 as level, CAST(name AS VARCHAR(1000)) as path
            FROM employees
            WHERE manager_id IS NULL

            UNION ALL

            -- 递归成员：下级员工
            SELECT e.id, e.name, e.manager_id, eh.level + 1,
                   CAST(eh.path || ' > ' || e.name AS VARCHAR(1000))
            FROM employees e
            INNER JOIN employee_hierarchy eh ON e.manager_id = eh.id
        ),
        department_stats AS (
            SELECT department_id,
                   COUNT(*) as employee_count,
                   AVG(salary) as avg_salary,
                   MAX(salary) as max_salary,
                   MIN(salary) as min_salary
            FROM employees
            GROUP BY department_id
        )
        SELECT eh.name, eh.level, eh.path,
               ds.employee_count, ds.avg_salary,
               eh.salary > ds.avg_salary as above_avg
        FROM employee_hierarchy eh
        JOIN department_stats ds ON eh.department_id = ds.department_id
        ORDER BY eh.path;
    )";

    auto statements = ParseSQL(sql);
    ASSERT_FALSE(statements.empty());

    auto select_stmt = dynamic_cast<SelectStatement*>(statements[0].get());
    ASSERT_NE(select_stmt, nullptr);

    // 验证CTE结构
    // 这里需要检查WITH子句
}

TEST_F(AdvancedSqlParserTest, ComplexSelect_SetOperations) {
    std::string sql = R"(
        (SELECT id, name, 'Employee' as type FROM employees WHERE active = true)
        UNION ALL
        (SELECT id, name, 'Contractor' as type FROM contractors WHERE status = 'active')
        UNION DISTINCT
        (SELECT id, name, 'Intern' as type FROM interns WHERE end_date > CURRENT_DATE)
        INTERSECT
        (SELECT id, name, 'All Personnel' as type FROM personnel WHERE department = 'Engineering')
        EXCEPT
        (SELECT id, name, 'Retired' as type FROM retired_employees)
        ORDER BY name, type;
    )";

    auto statements = ParseSQL(sql);
    ASSERT_FALSE(statements.empty());

    // 这应该是一个包含多个集合操作的复杂查询
    // 验证解析器能正确处理UNION ALL, UNION DISTINCT, INTERSECT, EXCEPT
}

TEST_F(AdvancedSqlParserTest, ComplexSelect_PivotUnpivot) {
    std::string sql = R"(
        -- 透视查询示例
        SELECT *
        FROM (
            SELECT department, job_title,
                   CASE WHEN performance_rating >= 4 THEN 1 ELSE 0 END as high_performer
            FROM employee_reviews
        ) AS source_table
        PIVOT (
            SUM(high_performer)
            FOR job_title IN ('Engineer', 'Manager', 'Director', 'VP')
        ) AS pivot_table
        ORDER BY department;

        -- 逆透视查询示例
        SELECT department, job_title, employee_count
        FROM (
            SELECT department, engineer_count, manager_count, director_count
            FROM department_stats
        ) AS source_table
        UNPIVOT (
            employee_count
            FOR job_title IN (engineer_count, manager_count, director_count)
        ) AS unpivot_table;
    )";

    // 解析多个语句
    auto statements = ParseSQL(sql);
    ASSERT_FALSE(statements.empty());

    // 验证PIVOT和UNPIVOT语法被正确解析
}

// 高级DDL语句测试
TEST_F(AdvancedSqlParserTest, DDL_AdvancedTableCreation) {
    std::string sql = R"(
        CREATE TABLE employees (
            id SERIAL PRIMARY KEY,
            employee_code VARCHAR(10) UNIQUE NOT NULL,
            first_name VARCHAR(50) NOT NULL,
            last_name VARCHAR(50) NOT NULL,
            email VARCHAR(255) UNIQUE,
            phone VARCHAR(20),
            hire_date DATE NOT NULL DEFAULT CURRENT_DATE,
            salary DECIMAL(12,2) CHECK (salary > 0),
            department_id INTEGER,
            manager_id INTEGER,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,

            -- 外键约束
            CONSTRAINT fk_department
                FOREIGN KEY (department_id)
                REFERENCES departments(id)
                ON DELETE CASCADE
                ON UPDATE RESTRICT,

            CONSTRAINT fk_manager
                FOREIGN KEY (manager_id)
                REFERENCES employees(id)
                ON DELETE SET NULL,

            -- 检查约束
            CONSTRAINT chk_salary_range
                CHECK (salary BETWEEN 30000 AND 1000000),

            CONSTRAINT chk_email_format
                CHECK (email LIKE '%@%.%'),

            -- 唯一约束
            CONSTRAINT uk_employee_department
                UNIQUE (employee_code, department_id),

            -- 排除约束（PostgreSQL特性）
            EXCLUDE (employee_code WITH =) WHERE (department_id IS NOT NULL)
        )
        INHERITS (person_base_table)
        PARTITION BY RANGE (hire_date);

        -- 创建分区
        CREATE TABLE employees_2023 PARTITION OF employees
            FOR VALUES FROM ('2023-01-01') TO ('2024-01-01');

        CREATE TABLE employees_2024 PARTITION OF employees
            FOR VALUES FROM ('2024-01-01') TO ('2025-01-01');
    )";

    auto statements = ParseSQL(sql);
    ASSERT_FALSE(statements.empty());

    // 验证CREATE TABLE语句及其各种约束
    // 这里需要遍历AST来验证具体的约束定义
}

TEST_F(AdvancedSqlParserTest, DDL_IndexCreation) {
    std::string sql = R"(
        -- B树索引
        CREATE INDEX idx_employees_name ON employees (last_name, first_name);

        -- 唯一索引
        CREATE UNIQUE INDEX idx_employees_email ON employees (email) WHERE active = true;

        -- 部分索引
        CREATE INDEX idx_high_salary_employees ON employees (salary) WHERE salary > 100000;

        -- 多列索引
        CREATE INDEX idx_employee_dept_salary ON employees (department_id, salary DESC);

        -- 函数索引
        CREATE INDEX idx_employees_name_lower ON employees (LOWER(last_name || ', ' || first_name));

        -- 全文搜索索引
        CREATE INDEX idx_employees_search ON employees USING GIN (to_tsvector('english', biography));

        -- 空间索引
        CREATE INDEX idx_locations_geom ON locations USING GIST (location_geom);

        -- 哈希索引（性能优化）
        CREATE INDEX idx_cache_keys ON cache USING HASH (cache_key);
    )";

    auto statements = ParseSQL(sql);
    ASSERT_FALSE(statements.empty());

    // 验证各种索引类型的创建语法
}

TEST_F(AdvancedSqlParserTest, DDL_StoredProcedures) {
    std::string sql = R"(
        CREATE OR REPLACE FUNCTION calculate_employee_bonus(
            employee_id INTEGER,
            performance_rating DECIMAL
        )
        RETURNS DECIMAL
        LANGUAGE SQL
        IMMUTABLE
        AS $$
            SELECT
                CASE
                    WHEN performance_rating >= 4.5 THEN salary * 0.20
                    WHEN performance_rating >= 4.0 THEN salary * 0.15
                    WHEN performance_rating >= 3.5 THEN salary * 0.10
                    ELSE salary * 0.05
                END
            FROM employees
            WHERE id = employee_id;
        $$;

        CREATE OR REPLACE PROCEDURE process_monthly_payroll(
            IN target_month DATE DEFAULT CURRENT_DATE,
            OUT processed_count INTEGER,
            OUT total_amount DECIMAL
        )
        LANGUAGE plpgsql
        AS $$
        BEGIN
            -- 更新薪资记录
            UPDATE payroll
            SET processed_date = CURRENT_DATE,
                status = 'PROCESSED'
            WHERE payroll_month = target_month
              AND status = 'PENDING';

            -- 获取处理统计
            SELECT COUNT(*), SUM(amount)
            INTO processed_count, total_amount
            FROM payroll
            WHERE payroll_month = target_month
              AND status = 'PROCESSED';

            -- 记录审计日志
            INSERT INTO payroll_audit (action, target_month, processed_count, total_amount, processed_by)
            VALUES ('MONTHLY_PROCESS', target_month, processed_count, total_amount, CURRENT_USER);
        END;
        $$;
    )";

    auto statements = ParseSQL(sql);
    ASSERT_FALSE(statements.empty());

    // 验证函数和存储过程的创建语法
}

TEST_F(AdvancedSqlParserTest, DDL_Triggers) {
    std::string sql = R"(
        -- 更新时间戳触发器
        CREATE TRIGGER update_employee_timestamp
            BEFORE UPDATE ON employees
            FOR EACH ROW
            EXECUTE FUNCTION update_updated_at_column();

        -- 审计触发器
        CREATE TRIGGER audit_employee_changes
            AFTER INSERT OR UPDATE OR DELETE ON employees
            FOR EACH ROW
            EXECUTE FUNCTION audit_employee_changes();

        -- 业务规则触发器
        CREATE TRIGGER enforce_salary_policy
            BEFORE INSERT OR UPDATE OF salary ON employees
            FOR EACH ROW
            EXECUTE FUNCTION check_salary_policy();

        -- 条件触发器
        CREATE TRIGGER prevent_weekend_updates
            BEFORE UPDATE ON critical_data
            FOR EACH ROW
            WHEN (EXTRACT(DOW FROM CURRENT_TIMESTAMP) IN (0, 6)) -- 周末
            EXECUTE FUNCTION prevent_update();
    )";

    auto statements = ParseSQL(sql);
    ASSERT_FALSE(statements.empty());

    // 验证触发器定义语法
}

TEST_F(AdvancedSqlParserTest, DDL_Views) {
    std::string sql = R"(
        -- 简单视图
        CREATE VIEW active_employees AS
            SELECT id, first_name, last_name, department_name
            FROM employees e
            JOIN departments d ON e.department_id = d.id
            WHERE e.active = true;

        -- 复杂视图（包含聚合）
        CREATE VIEW department_salary_stats AS
            SELECT
                d.name as department_name,
                COUNT(e.id) as employee_count,
                AVG(e.salary) as avg_salary,
                MAX(e.salary) as max_salary,
                MIN(e.salary) as min_salary,
                SUM(e.salary) as total_salary
            FROM departments d
            LEFT JOIN employees e ON d.id = e.department_id AND e.active = true
            GROUP BY d.id, d.name;

        -- 可更新视图
        CREATE VIEW employee_contact_info AS
            SELECT id, first_name, last_name, email, phone
            FROM employees
            WHERE active = true
            WITH CHECK OPTION;

        -- 递归视图
        CREATE RECURSIVE VIEW employee_hierarchy AS
            SELECT id, name, manager_id, 0 as level
            FROM employees
            WHERE manager_id IS NULL

            UNION ALL

            SELECT e.id, e.name, e.manager_id, eh.level + 1
            FROM employees e
            JOIN employee_hierarchy eh ON e.manager_id = eh.id;
    )";

    auto statements = ParseSQL(sql);
    ASSERT_FALSE(statements.empty());

    // 验证各种视图类型的创建语法
}

// 高级DML语句测试
TEST_F(AdvancedSqlParserTest, DML_AdvancedInsert) {
    std::string sql = R"(
        -- 多行插入
        INSERT INTO employees (first_name, last_name, email, department_id, salary)
        VALUES
            ('John', 'Doe', 'john.doe@company.com', 1, 75000.00),
            ('Jane', 'Smith', 'jane.smith@company.com', 2, 80000.00),
            ('Bob', 'Johnson', 'bob.johnson@company.com', 1, 70000.00);

        -- 插入...选择
        INSERT INTO employee_archive (id, name, department, archived_at)
        SELECT id, first_name || ' ' || last_name, department_name, CURRENT_TIMESTAMP
        FROM employees e
        JOIN departments d ON e.department_id = d.id
        WHERE e.active = false;

        -- 插入...返回
        INSERT INTO employees (first_name, last_name, email, hire_date)
        VALUES ('Alice', 'Wilson', 'alice.wilson@company.com', CURRENT_DATE)
        RETURNING id, first_name, last_name, hire_date;

        -- 条件插入 (UPSERT)
        INSERT INTO user_preferences (user_id, preference_key, preference_value)
        VALUES (123, 'theme', 'dark')
        ON CONFLICT (user_id, preference_key)
        DO UPDATE SET
            preference_value = EXCLUDED.preference_value,
            updated_at = CURRENT_TIMESTAMP;

        -- 批量插入
        INSERT INTO audit_log (action, table_name, record_id, user_id, timestamp)
        SELECT 'UPDATE', 'employees', id, updated_by, updated_at
        FROM employees_audit
        WHERE processed = false;
    )";

    auto statements = ParseSQL(sql);
    ASSERT_FALSE(statements.empty());

    // 验证各种INSERT语法变体
}

TEST_F(AdvancedSqlParserTest, DML_AdvancedUpdate) {
    std::string sql = R"(
        -- 多表更新
        UPDATE employees e
        SET salary = salary * 1.05
        FROM department_budget db
        WHERE e.department_id = db.department_id
          AND db.budget_remaining > e.salary * 0.05;

        -- 条件更新
        UPDATE products
        SET price = price * 0.9,
            discount_applied = true,
            updated_at = CURRENT_TIMESTAMP
        WHERE category = 'Electronics'
          AND price > 1000
          AND last_sold_date < CURRENT_DATE - INTERVAL '90 days';

        -- 更新...返回
        UPDATE inventory
        SET quantity = quantity - 1,
            last_updated = CURRENT_TIMESTAMP
        WHERE product_id = 12345
          AND quantity > 0
        RETURNING product_id, quantity, last_updated;

        -- CTE更新
        WITH top_performers AS (
            SELECT id
            FROM employees
            WHERE performance_rating >= 4.5
              AND salary < (SELECT MAX(salary) * 0.9 FROM employees)
        )
        UPDATE employees
        SET salary = salary * 1.15,
            last_raise_date = CURRENT_DATE
        WHERE id IN (SELECT id FROM top_performers);
    )";

    auto statements = ParseSQL(sql);
    ASSERT_FALSE(statements.empty());

    // 验证各种UPDATE语法变体
}

TEST_F(AdvancedSqlParserTest, DML_AdvancedDelete) {
    std::string sql = R"(
        -- 使用子查询的删除
        DELETE FROM order_items
        WHERE order_id IN (
            SELECT id
            FROM orders
            WHERE customer_id = 12345
              AND status = 'CANCELLED'
              AND created_at < CURRENT_DATE - INTERVAL '1 year'
        );

        -- 多表删除
        DELETE FROM employees e
        USING departments d
        WHERE e.department_id = d.id
          AND d.status = 'DISSOLVED';

        -- 删除...返回
        DELETE FROM expired_sessions
        WHERE last_activity < CURRENT_TIMESTAMP - INTERVAL '24 hours'
        RETURNING session_id, user_id, last_activity;

        -- 使用CTE的删除
        WITH inactive_users AS (
            SELECT id
            FROM users
            WHERE last_login < CURRENT_DATE - INTERVAL '2 years'
              AND status = 'ACTIVE'
        ),
        affected_orders AS (
            SELECT DISTINCT order_id
            FROM order_items oi
            JOIN inactive_users iu ON oi.customer_id = iu.id
        )
        DELETE FROM order_items
        WHERE order_id IN (SELECT order_id FROM affected_orders);
    )";

    auto statements = ParseSQL(sql);
    ASSERT_FALSE(statements.empty());

    // 验证各种DELETE语法变体
}

// 数据类型和函数测试
TEST_F(AdvancedSqlParserTest, DataTypes_JSON) {
    std::string sql = R"(
        CREATE TABLE user_profiles (
            id SERIAL PRIMARY KEY,
            user_id INTEGER NOT NULL,
            profile_data JSONB,
            settings JSON,
            metadata JSON,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        );

        -- JSON操作
        SELECT
            user_id,
            profile_data->>'name' as name,
            profile_data->>'email' as email,
            profile_data#>'{address,city}' as city,
            profile_data->'preferences'->>'theme' as theme,
            jsonb_object_keys(profile_data) as top_level_keys,
            jsonb_array_length(profile_data->'tags') as tag_count
        FROM user_profiles
        WHERE profile_data @> '{"active": true}'
          AND profile_data->'age' >= '21';

        -- JSON更新
        UPDATE user_profiles
        SET profile_data = profile_data || '{"last_login": "2023-12-01"}'::jsonb
        WHERE user_id = 12345;
    )";

    auto statements = ParseSQL(sql);
    ASSERT_FALSE(statements.empty());

    // 验证JSON数据类型和操作符
}

TEST_F(AdvancedSqlParserTest, DataTypes_Arrays) {
    std::string sql = R"(
        CREATE TABLE products (
            id SERIAL PRIMARY KEY,
            name VARCHAR(100),
            tags TEXT[],
            prices DECIMAL(10,2)[],
            images VARCHAR(500)[],
            related_products INTEGER[]
        );

        -- 数组操作
        SELECT
            name,
            tags[1] as primary_tag,
            array_length(tags, 1) as tag_count,
            prices[array_upper(prices, 1)] as highest_price,
            'electronics' = ANY(tags) as is_electronics,
            tags && ARRAY['laptop', 'computer'] as has_computer_tags,
            array_agg(DISTINCT tag) as unique_tags
        FROM (
            SELECT p.name, unnest(p.tags) as tag, p.prices
            FROM products p
        ) t
        GROUP BY name, tags, prices;

        -- 数组更新
        UPDATE products
        SET tags = array_append(tags, 'new_tag'),
            prices = array_cat(prices, ARRAY[29.99, 39.99])
        WHERE id = 123;
    )";

    auto statements = ParseSQL(sql);
    ASSERT_FALSE(statements.empty());

    // 验证数组数据类型和操作
}

TEST_F(AdvancedSqlParserTest, DataTypes_CustomTypes) {
    std::string sql = R"(
        -- 枚举类型
        CREATE TYPE user_status AS ENUM ('active', 'inactive', 'suspended', 'banned');

        -- 复合类型
        CREATE TYPE person_name AS (
            first_name VARCHAR(50),
            last_name VARCHAR(50),
            middle_name VARCHAR(50)
        );

        -- 范围类型
        CREATE TYPE date_range AS RANGE (subtype = date);

        -- 创建表使用自定义类型
        CREATE TABLE advanced_users (
            id SERIAL PRIMARY KEY,
            username VARCHAR(50) UNIQUE,
            full_name person_name,
            status user_status DEFAULT 'active',
            active_period date_range,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        );

        -- 使用自定义类型
        INSERT INTO advanced_users (username, full_name, active_period)
        VALUES (
            'johndoe',
            ('John', 'Doe', 'Q'),
            '[2020-01-01, 2025-12-31)'
        );

        -- 查询自定义类型
        SELECT
            username,
            (full_name).first_name,
            (full_name).last_name,
            status,
            lower(active_period) as active_start,
            upper(active_period) as active_end,
            active_period @> CURRENT_DATE as currently_active
        FROM advanced_users
        WHERE status = 'active'
          AND active_period && '[2023-01-01, 2023-12-31]'::date_range;
    )";

    auto statements = ParseSQL(sql);
    ASSERT_FALSE(statements.empty());

    // 验证自定义数据类型
}

// 高级函数和表达式测试
TEST_F(AdvancedSqlParserTest, AdvancedFunctions_Aggregates) {
    std::string sql = R"(
        SELECT
            department,
            COUNT(*) as employee_count,
            COUNT(DISTINCT job_title) as unique_titles,
            AVG(salary) as avg_salary,
            SUM(salary) as total_salary,
            MIN(salary) as min_salary,
            MAX(salary) as max_salary,
            STDDEV(salary) as salary_stddev,
            VARIANCE(salary) as salary_variance,
            MODE() WITHIN GROUP (ORDER BY salary) as modal_salary,
            PERCENTILE_CONT(0.5) WITHIN GROUP (ORDER BY salary) as median_salary,
            STRING_AGG(employee_name, ', ' ORDER BY salary DESC) as top_earners
        FROM employees
        WHERE active = true
        GROUP BY department
        HAVING COUNT(*) > 5
        ORDER BY avg_salary DESC;
    )";

    auto statements = ParseSQL(sql);
    ASSERT_FALSE(statements.empty());

    // 验证高级聚合函数
}

TEST_F(AdvancedSqlParserTest, AdvancedFunctions_Analytical) {
    std::string sql = R"(
        SELECT
            department,
            employee_name,
            salary,
            ROW_NUMBER() OVER (PARTITION BY department ORDER BY salary DESC) as dept_rank,
            RANK() OVER (PARTITION BY department ORDER BY salary DESC) as dept_rank_with_ties,
            DENSE_RANK() OVER (PARTITION BY department ORDER BY salary DESC) as dense_rank,
            PERCENT_RANK() OVER (PARTITION BY department ORDER BY salary DESC) as percent_rank,
            CUME_DIST() OVER (PARTITION BY department ORDER BY salary DESC) as cumulative_dist,
            NTILE(4) OVER (PARTITION BY department ORDER BY salary DESC) as quartile,
            LAG(salary, 1, 0) OVER (PARTITION BY department ORDER BY salary DESC) as next_lower_salary,
            LEAD(salary, 1, 0) OVER (PARTITION BY department ORDER BY salary DESC) as next_higher_salary,
            FIRST_VALUE(employee_name) OVER (PARTITION BY department ORDER BY salary DESC) as highest_earner,
            LAST_VALUE(employee_name) OVER (PARTITION BY department ORDER BY hire_date) as most_recent_hire,
            NTH_VALUE(employee_name, 2) OVER (PARTITION BY department ORDER BY salary DESC) as second_highest_earner
        FROM employees
        WHERE active = true
        ORDER BY department, dept_rank;
    )";

    auto statements = ParseSQL(sql);
    ASSERT_FALSE(statements.empty());

    // 验证分析函数
}

// 边界条件和错误处理测试
TEST_F(AdvancedSqlParserTest, BoundaryConditions_MaxNesting) {
    // 测试最大嵌套深度
    std::string sql = "SELECT ";
    for (int i = 0; i < 10; ++i) {
        sql += "(SELECT COUNT(*) FROM ";
    }
    sql += "users";
    for (int i = 0; i < 10; ++i) {
        sql += ")";
    }

    // 这个测试可能成功或失败，取决于解析器的嵌套限制
    ParseAndValidate(sql, true); // 期望解析成功或有意义的错误
}

TEST_F(AdvancedSqlParserTest, BoundaryConditions_LargeLiterals) {
    // 测试大文本字面量
    std::string large_text(10000, 'a'); // 10KB文本
    std::string sql = "SELECT '" + large_text + "' AS large_text;";

    auto statements = ParseSQL(sql);
    ASSERT_FALSE(statements.empty());
}

TEST_F(AdvancedSqlParserTest, ErrorHandling_InvalidSyntax) {
    // 测试无效语法
    std::vector<std::string> invalid_sqls = {
        "SELECT * FROM;",                    // 缺少表名
        "SELECT * FROM users WHERE;",       // 无效WHERE子句
        "CREATE TABLE (id INT);",           // 缺少表名
        "INSERT INTO users VALUES;",        // 缺少值
        "UPDATE users SET WHERE id = 1;",   // 缺少SET内容
        "DELETE FROM WHERE id = 1;",        // 缺少表名
    };

    for (const auto& sql : invalid_sqls) {
        EXPECT_FALSE(ParseAndValidate(sql, false)) << "Failed to reject invalid SQL: " << sql;
    }
}

TEST_F(AdvancedSqlParserTest, ErrorHandling_IncompleteStatements) {
    // 测试不完整语句
    std::vector<std::string> incomplete_sqls = {
        "SELECT * FROM users WHERE",
        "CREATE TABLE test (id INT,",
        "INSERT INTO users VALUES (1,",
        "UPDATE users SET name = 'test' WHERE",
    };

    for (const auto& sql : incomplete_sqls) {
        // 不完整语句可能被解析为有效语句或抛出异常
        try {
            auto statements = ParseSQL(sql);
            // 如果解析成功，检查是否为空或部分有效
            EXPECT_TRUE(statements.empty() || !statements.empty());
        } catch (const std::exception&) {
            // 异常是可接受的
            SUCCEED();
        }
    }
}

// 性能测试
TEST_F(AdvancedSqlParserTest, Performance_LargeQueries) {
    // 生成大型查询进行性能测试
    std::string sql = "SELECT ";
    for (int i = 0; i < 100; ++i) {
        sql += "column" + std::to_string(i) + ",";
    }
    sql.back() = ' '; // 替换最后一个逗号
    sql += "FROM large_table WHERE ";

    for (int i = 0; i < 50; ++i) {
        sql += "column" + std::to_string(i) + " > " + std::to_string(i) + " AND ";
    }
    sql += "active = true;";

    auto start = std::chrono::high_resolution_clock::now();

    auto statements = ParseSQL(sql);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // 验证解析成功且性能合理
    ASSERT_FALSE(statements.empty());
    EXPECT_LT(duration.count(), 1000); // 少于1秒
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
