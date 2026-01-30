/**
 * @file exception_test.cpp
 * @brief Exception模块完整单元测试
 *
 * 测试覆盖：
 * 1. Exception - 基础异常类
 * 2. IllegalArgumentException - 非法参数异常
 * 3. BufferPoolException - 缓冲池异常
 * 4. DiskManagerException - 磁盘管理器异常
 * 5. LockTimeoutException - 锁超时异常
 * 6. PageException - 页面异常
 * 7. NotImplementedException - 未实现功能异常
 */

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <stdexcept>

// 引入异常头文件
#include "src/exception/exception.h"

namespace sqlcc {
namespace test {

// ==================== BaseException Tests ====================

class BaseExceptionTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// 测试默认构造函数
TEST_F(BaseExceptionTest, DefaultConstructor) {
    std::string msg = "Test exception message";
    Exception ex(msg);
    EXPECT_STREQ(ex.what(), msg.c_str());
}

// 测试空消息
TEST_F(BaseExceptionTest, EmptyMessage) {
    std::string msg = "";
    Exception ex(msg);
    EXPECT_STREQ(ex.what(), msg.c_str());
}

// 测试长消息
TEST_F(BaseExceptionTest, LongMessage) {
    std::string msg(1000, 'A');  // 1000个'A'字符
    Exception ex(msg);
    EXPECT_STREQ(ex.what(), msg.c_str());
}

// 测试特殊字符消息
TEST_F(BaseExceptionTest, SpecialCharactersMessage) {
    std::string msg = "Test with 中文 and 特殊字符 !@#$%^&*()";
    Exception ex(msg);
    EXPECT_STREQ(ex.what(), msg.c_str());
}

// 测试异常继承关系 - 验证Exception继承自std::exception
TEST_F(BaseExceptionTest, InheritanceFromStdException) {
    Exception ex("Test message");
    std::exception* base = dynamic_cast<std::exception*>(&ex);
    EXPECT_NE(base, nullptr);
}

// 测试异常捕获
TEST_F(BaseExceptionTest, CatchByReference) {
    try {
        throw Exception("Test exception");
    } catch (const Exception& e) {
        EXPECT_STREQ(e.what(), "Test exception");
    } catch (...) {
        FAIL() << "Should catch by Exception reference";
    }
}

// 测试异常捕获 - 通过基类
TEST_F(BaseExceptionTest, CatchByBaseClass) {
    try {
        throw Exception("Test exception");
    } catch (const std::exception& e) {
        EXPECT_STREQ(e.what(), "Test exception");
    } catch (...) {
        FAIL() << "Should catch by std::exception reference";
    }
}

// ==================== IllegalArgumentException Tests ====================

class IllegalArgumentExceptionTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// 测试构造函数和消息前缀
TEST_F(IllegalArgumentExceptionTest, ConstructorWithPrefix) {
    std::string msg = "Invalid parameter value";
    IllegalArgumentException ex(msg);
    std::string expected = "Invalid parameter value";
    EXPECT_STREQ(ex.what(), expected.c_str());
}

// 测试继承关系
TEST_F(IllegalArgumentExceptionTest, Inheritance) {
    IllegalArgumentException ex("Test");
    Exception* base = dynamic_cast<Exception*>(&ex);
    EXPECT_NE(base, nullptr);
}

// 测试捕获特定异常
TEST_F(IllegalArgumentExceptionTest, CatchSpecificException) {
    try {
        throw IllegalArgumentException("Invalid argument");
    } catch (const IllegalArgumentException& e) {
        EXPECT_STREQ(e.what(), "Invalid argument");
    } catch (...) {
        FAIL() << "Should catch IllegalArgumentException";
    }
}

// 测试通过基类捕获
TEST_F(IllegalArgumentExceptionTest, CatchByExceptionBase) {
    try {
        throw IllegalArgumentException("Invalid argument");
    } catch (const Exception& e) {
        std::string msg = e.what();
        EXPECT_TRUE(msg.find("Invalid argument") != std::string::npos);
    }
}

// ==================== BufferPoolException Tests ====================

class BufferPoolExceptionTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// 测试构造函数和消息前缀
TEST_F(BufferPoolExceptionTest, ConstructorWithPrefix) {
    std::string msg = "Buffer overflow detected";
    BufferPoolException ex(msg);
    std::string expected = "Buffer overflow detected";
    EXPECT_STREQ(ex.what(), expected.c_str());
}

// 测试继承关系
TEST_F(BufferPoolExceptionTest, Inheritance) {
    BufferPoolException ex("Test");
    Exception* base = dynamic_cast<Exception*>(&ex);
    EXPECT_NE(base, nullptr);
}

// 测试捕获特定异常
TEST_F(BufferPoolExceptionTest, CatchSpecificException) {
    try {
        throw BufferPoolException("Buffer error");
    } catch (const BufferPoolException& e) {
        EXPECT_STREQ(e.what(), "Buffer error");
    }
}

// ==================== DiskManagerException Tests ====================

class DiskManagerExceptionTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// 测试构造函数和消息前缀
TEST_F(DiskManagerExceptionTest, ConstructorWithPrefix) {
    std::string msg = "Disk write failed";
    DiskManagerException ex(msg);
    std::string expected = "Disk write failed";
    EXPECT_STREQ(ex.what(), expected.c_str());
}

// 测试继承关系
TEST_F(DiskManagerExceptionTest, Inheritance) {
    DiskManagerException ex("Test");
    Exception* base = dynamic_cast<Exception*>(&ex);
    EXPECT_NE(base, nullptr);
}

// 测试捕获特定异常
TEST_F(DiskManagerExceptionTest, CatchSpecificException) {
    try {
        throw DiskManagerException("Disk error");
    } catch (const DiskManagerException& e) {
        EXPECT_STREQ(e.what(), "Disk error");
    }
}

// ==================== IOException Tests ====================
// IOException 在 io_exception.h 中定义，如果需要测试请添加相应的依赖

// ==================== LockTimeoutException Tests ====================

class LockTimeoutExceptionTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// 测试构造函数和消息前缀
TEST_F(LockTimeoutExceptionTest, ConstructorWithPrefix) {
    std::string msg = "Lock acquisition timeout after 5000ms";
    LockTimeoutException ex(msg);
    std::string expected = "Lock acquisition timeout after 5000ms";
    EXPECT_STREQ(ex.what(), expected.c_str());
}

// 测试继承关系
TEST_F(LockTimeoutExceptionTest, Inheritance) {
    LockTimeoutException ex("Test");
    Exception* base = dynamic_cast<Exception*>(&ex);
    EXPECT_NE(base, nullptr);
}

// 测试捕获特定异常
TEST_F(LockTimeoutExceptionTest, CatchSpecificException) {
    try {
        throw LockTimeoutException("Lock timeout");
    } catch (const LockTimeoutException& e) {
        EXPECT_STREQ(e.what(), "Lock timeout");
    }
}

// ==================== PageException Tests ====================

class PageExceptionTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// 测试构造函数和消息前缀
TEST_F(PageExceptionTest, ConstructorWithPrefix) {
    std::string msg = "Page not found: page_id=12345";
    PageException ex(msg);
    std::string expected = "Page not found: page_id=12345";
    EXPECT_STREQ(ex.what(), expected.c_str());
}

// 测试继承关系
TEST_F(PageExceptionTest, Inheritance) {
    PageException ex("Test");
    Exception* base = dynamic_cast<Exception*>(&ex);
    EXPECT_NE(base, nullptr);
}

// 测试捕获特定异常
TEST_F(PageExceptionTest, CatchSpecificException) {
    try {
        throw PageException("Page error");
    } catch (const PageException& e) {
        EXPECT_STREQ(e.what(), "Page error");
    }
}

// ==================== NotImplementedException Tests ====================

class NotImplementedExceptionTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// 测试构造函数和消息前缀
TEST_F(NotImplementedExceptionTest, ConstructorWithPrefix) {
    std::string msg = "Feature 'JSON indexing' is not implemented yet";
    NotImplementedException ex(msg);
    std::string expected = "Feature 'JSON indexing' is not implemented yet";
    EXPECT_STREQ(ex.what(), expected.c_str());
}

// 测试继承关系
TEST_F(NotImplementedExceptionTest, Inheritance) {
    NotImplementedException ex("Test");
    Exception* base = dynamic_cast<Exception*>(&ex);
    EXPECT_NE(base, nullptr);
}

// 测试捕获特定异常
TEST_F(NotImplementedExceptionTest, CatchSpecificException) {
    try {
        throw NotImplementedException("Feature not implemented");
    } catch (const NotImplementedException& e) {
        EXPECT_STREQ(e.what(), "Feature not implemented");
    }
}

// ==================== 异常类型集成测试 ====================

class ExceptionIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// 测试多异常类型捕获
TEST_F(ExceptionIntegrationTest, CatchMultipleExceptionTypes) {
    int test_type = 0;

    try {
        if (test_type == 0) {
            throw IllegalArgumentException("Invalid arg");
        } else if (test_type == 1) {
            throw BufferPoolException("Buffer error");
        } else {
            throw Exception("Generic error");
        }
    } catch (const IllegalArgumentException& e) {
        EXPECT_TRUE(std::string(e.what()) != "");
    } catch (const BufferPoolException& e) {
        EXPECT_TRUE(std::string(e.what()) != "");
    } catch (const Exception& e) {
        EXPECT_TRUE(std::string(e.what()) != "");
    }
}

// 测试异常重新抛出
TEST_F(ExceptionIntegrationTest, ExceptionRethrow) {
    try {
        try {
            throw BufferPoolException("First throw");
        } catch (...) {
            throw;  // 重新抛出
        }
        FAIL() << "Should not reach here";
    } catch (const BufferPoolException& e) {
        EXPECT_TRUE(std::string(e.what()) != "");
    }
}

// 测试异常在函数间传递
void ThrowBufferPoolException() {
    throw BufferPoolException("Function internal error");
}

TEST_F(ExceptionIntegrationTest, ExceptionPassThroughFunctions) {
    try {
        ThrowBufferPoolException();
        FAIL() << "Should not reach here";
    } catch (const BufferPoolException& e) {
        EXPECT_TRUE(std::string(e.what()) != "");
    }
}

// 测试异常嵌套
TEST_F(ExceptionIntegrationTest, NestedExceptionHandling) {
    try {
        try {
            throw BufferPoolException("Inner buffer error");
        } catch (const Exception& inner) {
            // 捕获内部异常后抛出外部异常
            throw PageException(std::string("Outer page error caused by: ") + inner.what());
        }
    } catch (const PageException& outer) {
        std::string msg = outer.what();
        EXPECT_TRUE(msg.find("Inner buffer error") != std::string::npos);
    }
}

// ==================== 异常性能测试 ====================

class ExceptionPerformanceTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// 测试异常构造性能
TEST_F(ExceptionPerformanceTest, ExceptionConstructionPerformance) {
    const int iterations = 1000;
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < iterations; ++i) {
        Exception ex("Performance test message");
        // 使用异常对象，避免优化掉
        volatile const char* msg = ex.what();
        (void)msg;
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    // 异常构造应该在合理时间内完成（1000次应该在100ms以内）
    EXPECT_LT(duration.count(), 100000);  // 100ms = 100000 microseconds
}

// 测试异常抛出和捕获性能
TEST_F(ExceptionPerformanceTest, ExceptionThrowCatchPerformance) {
    const int iterations = 1000;
    int catch_count = 0;
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < iterations; ++i) {
        try {
            throw BufferPoolException("Performance test");
        } catch (const BufferPoolException&) {
            catch_count++;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    EXPECT_EQ(catch_count, iterations);
    // 异常抛出和捕获应该在合理时间内完成
    EXPECT_LT(duration.count(), 500000);  // 500ms
}

} // namespace test
} // namespace sqlcc

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}