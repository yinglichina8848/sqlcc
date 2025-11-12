include_directories(${CMAKE_SOURCE_DIR}/include)

# 定义测试源文件列表
set(TEST_SOURCES
    storage_engine_test.cc
    buffer_pool_test.cc
    storage_engine_newpage_test.cc
    disk_manager_test.cc
)

# 定义增强测试源文件列表
set(ENHANCED_TEST_SOURCES
    unit/config_manager_test.cc
    unit/config_manager_enhanced_test.cc
    unit/buffer_pool_enhanced_test.cc
    unit/page_enhanced_test.cc
    unit/disk_manager_enhanced_test.cc
    unit/storage_engine_enhanced_test.cc
)

# 创建标准测试可执行文件的函数
function(add_sqlcc_test test_name)
    add_executable(${test_name} ${test_name}.cc)
    
    # 统一设置包含目录和链接库
    target_include_directories(${test_name} PRIVATE ${CMAKE_SOURCE_DIR}/include)
    target_link_libraries(${test_name} 
        sqlcc_core
        GTest::gtest 
        GTest::gtest_main
        Threads::Threads
    )
    
    # 注册测试
    add_test(NAME ${test_name} COMMAND ${test_name})
    
    # 设置测试超时时间（秒）
    set_tests_properties(${test_name} PROPERTIES TIMEOUT 300)
endfunction()

# 创建增强测试可执行文件的函数
function(add_sqlcc_enhanced_test test_name test_file)
    add_executable(${test_name} ${test_file})
    
    # 统一设置包含目录和链接库
    target_include_directories(${test_name} PRIVATE ${CMAKE_SOURCE_DIR}/include)
    target_link_libraries(${test_name} 
        sqlcc_core
        GTest::gtest 
        GTest::gtest_main
        Threads::Threads
    )
    
    # 注册测试
    add_test(NAME ${test_name} COMMAND ${test_name})
    
    # 设置测试超时时间（秒）
    set_tests_properties(${test_name} PROPERTIES TIMEOUT 300)
endfunction()

# 为每个标准测试源文件创建测试可执行文件
set(TEST_TARGETS "")
foreach(test_source ${TEST_SOURCES})
    # 从文件名中提取测试名称（去掉.cc后缀）
    get_filename_component(test_name ${test_source} NAME_WE)
    add_sqlcc_test(${test_name})
    list(APPEND TEST_TARGETS ${test_name})
endforeach()

# 为每个增强测试源文件创建测试可执行文件
set(ENHANCED_TEST_TARGETS "")
foreach(test_source ${ENHANCED_TEST_SOURCES})
    # 从文件名中提取测试名称（去掉.cc后缀）
    get_filename_component(test_name ${test_source} NAME_WE)
    add_sqlcc_enhanced_test(${test_name} ${test_source})
    list(APPEND ENHANCED_TEST_TARGETS ${test_name})
endforeach()

# 创建测试组
set_tests_properties(
    storage_engine_test buffer_pool_test 
    storage_engine_newpage_test disk_manager_test
    config_manager_test config_manager_enhanced_test
    buffer_pool_enhanced_test page_enhanced_test 
    disk_manager_enhanced_test storage_engine_enhanced_test
    PROPERTIES LABELS "unit_tests"
)

# 创建单元测试分类目标
add_custom_target(unit_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --label-regex unit_tests
    DEPENDS ${TEST_TARGETS} ${ENHANCED_TEST_TARGETS}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Run all unit tests"
)

# 创建运行单个单元测试的目标
add_custom_target(run_storage_engine_test
    COMMAND $<TARGET_FILE:storage_engine_test>
    DEPENDS storage_engine_test
    COMMENT "Run storage engine test"
)

add_custom_target(run_buffer_pool_test
    COMMAND $<TARGET_FILE:buffer_pool_test>
    DEPENDS buffer_pool_test
    COMMENT "Run buffer pool test"
)

add_custom_target(run_disk_manager_test
    COMMAND $<TARGET_FILE:disk_manager_test>
    DEPENDS disk_manager_test
    COMMENT "Run disk manager test"
)

# Add performance tests
add_subdirectory(performance)
