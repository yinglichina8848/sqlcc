# 独立CRUD性能测试CMake配置
# 避免主项目链接错误，仅编译和测试CRUD性能测试

# 启用测试
enable_testing()

# 查找Google Test库
find_package(GTest REQUIRED)

# 包含头文件目录
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# 添加CRUD性能测试文件
set(CRUD_PERFORMANCE_TESTS
    ../crud_performance_test.cc
    ../crud_performance_test_main.cc
)

# 创建CRUD性能测试可执行文件
add_executable(crud_performance_test_standalone ${CRUD_PERFORMANCE_TESTS})

# 链接必要的库（仅链接核心库，避免链接错误）
target_link_libraries(crud_performance_test_standalone 
    sqlcc_core
    sqlcc_parser
    GTest::gtest
    GTest::gtest_main
    Threads::Threads
)

# 设置编译选项
target_compile_options(crud_performance_test_standalone PRIVATE
    $<$<CONFIG:Debug>:-O0 -g>
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
)

# 设置输出目录
set_target_properties(crud_performance_test_standalone PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 创建运行CRUD性能测试的目标
add_custom_target(run_crud_performance_standalone
    COMMAND ${CMAKE_BINARY_DIR}/bin/crud_performance_test_standalone --all --verbose
    DEPENDS crud_performance_test_standalone
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Run standalone CRUD performance tests"
)

# 创建快速CRUD性能测试目标
add_custom_target(run_crud_quick_standalone
    COMMAND ${CMAKE_BINARY_DIR}/bin/crud_performance_test_standalone --quick --verbose
    DEPENDS crud_performance_test_standalone
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Run quick standalone CRUD performance tests"
)