# 配置管理器单元测试CMake配置

# 启用测试
enable_testing()

# 查找Google Test库
find_package(GTest REQUIRED)

# 包含头文件目录
include_directories(${CMAKE_SOURCE_DIR}/include)

# 创建配置管理器测试可执行文件
add_executable(config_manager_test 
    unit/config_manager_test.cc
)

# 链接必要的库
target_link_libraries(config_manager_test 
    sqlcc_core
    GTest::gtest 
    GTest::gtest_main
    pthread
)

# 注册配置管理器测试
add_test(NAME ConfigManagerTest COMMAND config_manager_test)

# 创建配置管理器增强测试可执行文件
add_executable(config_manager_enhanced_test 
    unit/config_manager_enhanced_test.cc
)

# 链接必要的库
target_link_libraries(config_manager_enhanced_test 
    sqlcc_core
    GTest::gtest 
    GTest::gtest_main
    pthread
)

# 注册配置管理器增强测试
add_test(NAME ConfigManagerEnhancedTest COMMAND config_manager_enhanced_test)

# 创建存储引擎测试可执行文件
add_executable(storage_engine_test storage_engine_test.cc)

# 链接必要的库
target_link_libraries(storage_engine_test 
    sqlcc_core
    GTest::gtest 
    GTest::gtest_main
    pthread
)

# 注册存储引擎测试
add_test(NAME StorageEngineTest COMMAND storage_engine_test)

# Add enhanced disk manager tests
add_executable(disk_manager_enhanced_test unit/disk_manager_enhanced_test.cc)
target_link_libraries(disk_manager_enhanced_test 
    sqlcc_core 
    GTest::gtest 
    GTest::gtest_main 
    pthread
)

# Add enhanced buffer pool tests
add_executable(buffer_pool_enhanced_test unit/buffer_pool_enhanced_test.cc)
target_link_libraries(buffer_pool_enhanced_test 
    sqlcc_core 
    GTest::gtest 
    GTest::gtest_main 
    pthread
)

# Register enhanced buffer pool tests
add_test(NAME BufferPoolEnhancedTest COMMAND buffer_pool_enhanced_test)
add_test(NAME DiskManagerEnhancedTest COMMAND disk_manager_enhanced_test)

# Page enhanced test
add_executable(page_enhanced_test unit/page_enhanced_test.cc)
target_link_libraries(page_enhanced_test 
    sqlcc_core 
    GTest::gtest 
    GTest::gtest_main 
    pthread
)

# Register enhanced page tests
add_test(NAME PageEnhancedTest COMMAND page_enhanced_test)

# Add enhanced storage engine tests
add_executable(storage_engine_enhanced_test unit/storage_engine_enhanced_test.cc)
target_link_libraries(storage_engine_enhanced_test 
    sqlcc_core 
    GTest::gtest 
    GTest::gtest_main 
    pthread
)

# Register enhanced storage engine tests
add_test(NAME StorageEngineEnhancedTest COMMAND storage_engine_enhanced_test)

# Add performance tests
add_subdirectory(performance)