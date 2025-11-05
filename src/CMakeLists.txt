# 添加库定义
add_library(sqlcc_core STATIC
    buffer_pool.cc
    disk_manager.cc
    page.cc
    storage_engine.cc
)

# 设置包含目录
target_include_directories(sqlcc_core PUBLIC ${CMAKE_SOURCE_DIR}/include)
