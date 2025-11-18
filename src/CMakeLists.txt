# 创建SQL parser独立库
add_library(sqlcc_parser STATIC
    sql_parser/token.cpp
    sql_parser/ast_node.cpp
    sql_parser/ast_nodes.cpp
    sql_parser/lexer.cpp
    sql_parser/parser.cpp
)

# 设置SQL parser库的包含目录
target_include_directories(sqlcc_parser PUBLIC ${CMAKE_SOURCE_DIR}/include)

# 添加核心库定义
add_library(sqlcc_core STATIC
    buffer_pool.cc
    config_manager.cc
    data_type.cpp
    disk_manager.cc
    page.cc
    storage_engine.cc
    index_manager.cpp
    b_plus_tree.cc
    transaction_manager.cc
    # sqlcc/wal_manager.cc  # v0.4.8暂时移除，后续集成
)

# 设置包含目录
target_include_directories(sqlcc_core PUBLIC ${CMAKE_SOURCE_DIR}/include)

# 创建SQL执行器库
add_library(sqlcc_executor STATIC
    constraint_executor.cpp
    sql_executor.cpp
)

# 设置SQL执行器库的包含目录
target_include_directories(sqlcc_executor PUBLIC ${CMAKE_SOURCE_DIR}/include)

# 添加线程库依赖（如果项目需要）
find_package(Threads)
if(Threads_FOUND)
    target_link_libraries(sqlcc_parser PUBLIC Threads::Threads)
    target_link_libraries(sqlcc_core PUBLIC Threads::Threads)
endif()

# 核心库链接到SQL parser库
target_link_libraries(sqlcc_core PUBLIC sqlcc_parser)

# SQL执行器库链接到核心库和parser库
target_link_libraries(sqlcc_executor PUBLIC sqlcc_core)
target_link_libraries(sqlcc_executor PUBLIC sqlcc_parser)

# 添加可执行文件
add_executable(sqlcc_main main.cc)
add_executable(config_test config_test.cc)

# 添加交互式SQL工具
add_executable(isql isql_main.cc)
# 链接库
target_link_libraries(sqlcc_main sqlcc_core)
target_link_libraries(config_test sqlcc_core)
target_link_libraries(isql sqlcc_executor sqlcc_parser)

# 查找readline库（用于命令行补全和历史功能，如果可用）
find_package(PkgConfig)
pkg_check_modules(READLINE QUIET readline)
if(READLINE_FOUND)
    target_include_directories(isql PRIVATE ${READLINE_INCLUDE_DIRS})
    target_link_libraries(isql ${READLINE_LIBRARIES})
    target_compile_definitions(isql PRIVATE USE_READLINE=1)
else()
    message(WARNING "Readline library not found, using basic command line input")
endif()

# 设置可执行文件输出目录
set_target_properties(sqlcc_main PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

set_target_properties(config_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

set_target_properties(isql PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 安装规则
install(TARGETS sqlcc_main DESTINATION bin)
install(TARGETS config_test DESTINATION bin)
install(TARGETS isql DESTINATION bin)
install(FILES ${CMAKE_SOURCE_DIR}/config/sqlcc.conf DESTINATION etc)
