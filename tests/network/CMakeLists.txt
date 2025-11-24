# 网络测试的CMake配置

# 设置测试源文件
set(NETWORK_TEST_SOURCES
    encryption_test.cc
    network_encryption_integration_test.cc
    server_client_encryption_test.cc
    database_connection_test.cc
)

# 创建测试可执行文件
add_executable(network_encryption_tests ${NETWORK_TEST_SOURCES})

# 链接必要的库
target_link_libraries(network_encryption_tests
    PRIVATE
    gtest
    gtest_main
    sqlcc_core
    network
    pthread
)

# 添加测试
add_test(NAME NetworkEncryptionTests COMMAND network_encryption_tests)
