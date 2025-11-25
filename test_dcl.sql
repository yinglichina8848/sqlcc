-- 测试DCL操作：创建用户、授权和撤销权限

-- 创建新用户
CREATE USER test_user IDENTIFIED BY 'password123';

-- 授予用户所有权限
GRANT ALL PRIVILEGES TO test_user;

-- 查看用户权限
SHOW GRANTS FOR test_user;

-- 撤销部分权限
REVOKE INSERT, UPDATE, DELETE ON *.* FROM test_user;

-- 再次查看用户权限
SHOW GRANTS FOR test_user;

-- 删除用户
DROP USER test_user;

-- 确认用户已删除
SHOW USERS;