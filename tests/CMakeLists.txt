# 启用测试
enable_testing()

# 查找Google Test库
find_package(GTest REQUIRED)

# 包含头文件目录
include_directories(${CMAKE_SOURCE_DIR}/include)

# 创建测试可执行文件
add_executable(storage_engine_test storage_engine_test.cc)

# 链接必要的库
target_link_libraries(storage_engine_test 
    sqlcc_core
    GTest::gtest 
    GTest::gtest_main
    pthread
)

# 注册测试
add_test(NAME StorageEngineTest COMMAND storage_engine_test)