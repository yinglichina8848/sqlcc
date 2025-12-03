#include <gtest/gtest.h>
#include <iostream>
#include <fstream>
#include <string>
#include "../include/sql_executor.h"

// DCL测试

TEST(DCLTest, SqlExecutorConstruction) {
    // 测试SqlExecutor构造
    sqlcc::SqlExecutor executor;
    SUCCEED();
}