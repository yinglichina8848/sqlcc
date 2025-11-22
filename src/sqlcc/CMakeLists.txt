# Add source files for the sqlcc library
set(SQLCC_SRCS
    wal_manager.cc
    network.cc  # 添加网络模块源文件
)

add_library(sqlcc ${SQLCC_SRCS})

target_include_directories(sqlcc PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(sqlcc PUBLIC
    pthread
)