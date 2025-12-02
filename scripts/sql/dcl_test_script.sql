-- DCL（数据控制语言）测试脚本
-- 包含GRANT、REVOKE、用户管理和角色管理语句的测试用例

-- 测试创建用户
CREATE USER test_user IDENTIFIED BY 'test_password';
CREATE USER admin_user IDENTIFIED BY 'admin_password';

-- 测试ALTER USER命令
-- 修改用户密码
ALTER USER test_user IDENTIFIED BY 'new_password';
-- 修改用户角色
ALTER USER test_user SET ROLE ADMIN;

-- 测试角色管理命令
-- 创建角色
CREATE ROLE developer;
CREATE ROLE analyst;
CREATE ROLE manager;

-- 修改角色
ALTER ROLE developer RENAME TO senior_developer;

-- 测试SET ROLE命令
SET ROLE senior_developer;

-- 测试GRANT语句
-- 授予用户对特定表的SELECT权限
GRANT SELECT ON users TO test_user;

-- 授予用户对特定表的多个权限
GRANT INSERT, UPDATE ON users TO test_user;

-- 授予管理员用户对所有表的所有权限
GRANT ALL PRIVILEGES ON *.* TO admin_user;

-- 测试REVOKE语句
-- 撤销用户的UPDATE权限
REVOKE UPDATE ON users FROM test_user;

-- 撤销用户的所有权限
REVOKE ALL PRIVILEGES ON users FROM test_user;

-- 测试复杂的权限组合
GRANT SELECT, INSERT ON products TO test_user;
GRANT DELETE ON orders TO test_user;

-- 验证权限状态
SHOW GRANTS FOR test_user;
SHOW GRANTS FOR admin_user;

-- 清理测试数据
-- 删除角色
DROP ROLE senior_developer;
DROP ROLE analyst;
DROP ROLE manager;

-- 删除用户
DROP USER test_user;
DROP USER admin_user;

-- 测试结束标记
SELECT 'DCL测试完成' AS test_result;