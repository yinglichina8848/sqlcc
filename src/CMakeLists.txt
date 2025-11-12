# 添加库定义
add_library(sqlcc_core STATIC
    buffer_pool.cc
    config_manager.cc
    disk_manager.cc
    page.cc
    storage_engine.cc
)

# 设置包含目录
target_include_directories(sqlcc_core PUBLIC ${CMAKE_SOURCE_DIR}/include)

# 添加线程库依赖（如果项目需要）
find_package(Threads)
if(Threads_FOUND)
    target_link_libraries(sqlcc_core PUBLIC Threads::Threads)
endif()

# 添加可执行文件
add_executable(sqlcc_main main.cc)
add_executable(config_test config_test.cc)
# 链接库
target_link_libraries(sqlcc_main sqlcc_core)
target_link_libraries(config_test sqlcc_core)

# 设置可执行文件输出目录
set_target_properties(sqlcc_main PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

set_target_properties(config_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 安装规则
install(TARGETS sqlcc_main DESTINATION bin)
install(TARGETS config_test DESTINATION bin)
install(FILES ${CMAKE_SOURCE_DIR}/config/sqlcc.conf DESTINATION etc)
