# CRUD性能测试CMake配置

# 设置包含目录
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/core
    ${CMAKE_SOURCE_DIR}/include/sql
    ${CMAKE_SOURCE_DIR}/include/utils
    ${CMAKE_SOURCE_DIR}/tests/performance
    ${CMAKE_SOURCE_DIR}/tests/performance/basic
)

# 添加CRUD性能测试可执行文件
add_executable(crud_performance_test
    crud_performance_main.cpp
    real_crud_performance_test.cpp
)

# 链接依赖库
target_link_libraries(crud_performance_test
    PRIVATE
    gtest
    gtest_main
    pthread
    ${CMAKE_DL_LIBS}
    sqlcc_core_lib
    sqlcc_parser
    sqlcc_config_manager
    sqlcc_storage_engine
    sqlcc_transaction_manager
    sqlcc_executor
    performance_test_base
)

# 设置编译选项
target_compile_options(crud_performance_test PRIVATE -O2 -DNDEBUG)

# 安装目标（可选）
if(BUILD_PERFORMANCE_TESTS)
    install(TARGETS crud_performance_test
        RUNTIME DESTINATION bin/tests/performance
    )
endif()