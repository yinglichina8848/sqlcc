"""全面测试套件定义"""

# 定义所有测试套件的标签
ALL_TEST_SUITES = [
    # 单元测试
    "//tests/unit/executor:task_executor_test",
    "//tests/unit/parser:parser_unit_tests",
    "//tests/unit/basic:basic_unit_tests",
    
    # 组件测试
    "//tests/components/network:network_component_tests",
    "//tests/components/storage:storage_component_tests",
    "//tests/components/parser:parser_component_tests",
    "//tests/components/executor:executor_component_tests",
    "//tests/components/security:security_component_tests",
    
    # 集成测试
    "//tests/integration:integration_tests",
    
    # 网络测试
    "//tests/network:network_unit_tests",
    "//tests/network:network_comprehensive_tests",
    
    # 存储引擎测试
    "//tests/storage_engine:storage_engine_tests",
    
    # SQL解析器测试
    "//tests/sql_parser:sql_parser_core_tests",
    
    # SQL执行器测试
    "//tests/sql_executor:sql_executor_tests",
    
    # 客户端服务器测试
    "//tests/client_server:client_server_test_suite",
    
    # 高级SQL测试
    "//tests/advanced_sql:advanced_sql_test_suite",
]

# 定义核心测试套件（用于快速测试）
CORE_TEST_SUITES = [
    # 核心组件测试
    "//tests/components/network:network_component_tests",
    "//tests/components/storage:storage_component_tests",
    "//tests/network:network_unit_tests",
    "//tests/unit/basic:basic_unit_tests",
]