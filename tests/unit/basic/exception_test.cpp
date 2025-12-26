/**
 * @file exception_test.cpp
 * @brief 异常处理类单元测试
 *
 * 测试SQLCC系统中所有异常类的基本功能，包括：
 * - 异常类的继承关系
 * - 异常消息传递
 * - 异常类型的正确性
 * - 异常的抛出和捕获
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <stdexcept>
#include <string>
#include <memory>

#include "exception.h"

// 使用sqlcc命名空间
using namespace sqlcc;

/**
 * @brief Exception测试套件
 */
class ExceptionTest : public ::testing::Test {
protected:
    /**
     * @brief 测试前设置
     */
    void SetUp() override {
        // 不需要特殊设置
    }

    /**
     * @brief 测试后清理
     */
    void TearDown() override {
        // 不需要特殊清理
    }
};

/**
 * @brief 测试基类Exception
 */
TEST_F(ExceptionTest, BaseException) {
    // 测试构造函数和消息传递
    std::string test_message = "Test exception message";
    Exception ex(test_message);

    // 验证异常消息
    EXPECT_STREQ(ex.what(), test_message.c_str());

    // 验证继承关系
    EXPECT_TRUE(dynamic_cast<std::runtime_error*>(&ex) != nullptr);

    // 测试空消息
    Exception empty_ex("");
    EXPECT_STREQ(empty_ex.what(), "");

    // 测试长消息
    std::string long_message(1000, 'A');
    Exception long_ex(long_message);
    EXPECT_STREQ(long_ex.what(), long_message.c_str());
}

/**
 * @brief 测试IOException
 */
TEST_F(ExceptionTest, IOException) {
    // 测试构造函数
    std::string io_message = "File I/O error occurred";
    IOException io_ex(io_message);

    // 验证异常消息
    EXPECT_STREQ(io_ex.what(), io_message.c_str());

    // 验证继承关系
    EXPECT_TRUE(dynamic_cast<Exception*>(&io_ex) != nullptr);
    EXPECT_TRUE(dynamic_cast<std::runtime_error*>(&io_ex) != nullptr);

    // 测试抛出和捕获
    try {
        throw IOException("Test I/O exception");
    } catch (const IOException& e) {
        EXPECT_STREQ(e.what(), "Test I/O exception");
    } catch (...) {
        FAIL() << "Expected IOException to be caught";
    }
}

/**
 * @brief 测试BufferPoolException
 */
TEST_F(ExceptionTest, BufferPoolException) {
    // 测试构造函数
    std::string buffer_message = "Buffer pool allocation failed";
    BufferPoolException buffer_ex(buffer_message);

    // 验证异常消息
    EXPECT_STREQ(buffer_ex.what(), buffer_message.c_str());

    // 验证继承关系
    EXPECT_TRUE(dynamic_cast<Exception*>(&buffer_ex) != nullptr);
    EXPECT_TRUE(dynamic_cast<std::runtime_error*>(&buffer_ex) != nullptr);

    // 测试抛出和捕获
    try {
        throw BufferPoolException("Buffer pool exhausted");
    } catch (const BufferPoolException& e) {
        EXPECT_STREQ(e.what(), "Buffer pool exhausted");
    } catch (const Exception& e) {
        // 也可以用基类捕获
        EXPECT_STREQ(e.what(), "Buffer pool exhausted");
    } catch (...) {
        FAIL() << "Expected BufferPoolException to be caught";
    }
}

/**
 * @brief 测试PageException
 */
TEST_F(ExceptionTest, PageException) {
    // 测试构造函数
    std::string page_message = "Page read/write error";
    PageException page_ex(page_message);

    // 验证异常消息
    EXPECT_STREQ(page_ex.what(), page_message.c_str());

    // 验证继承关系
    EXPECT_TRUE(dynamic_cast<Exception*>(&page_ex) != nullptr);
    EXPECT_TRUE(dynamic_cast<std::runtime_error*>(&page_ex) != nullptr);

    // 测试抛出和捕获
    try {
        throw PageException("Invalid page access");
    } catch (const PageException& e) {
        EXPECT_STREQ(e.what(), "Invalid page access");
    } catch (...) {
        FAIL() << "Expected PageException to be caught";
    }
}

/**
 * @brief 测试DiskManagerException
 */
TEST_F(ExceptionTest, DiskManagerException) {
    // 测试构造函数
    std::string disk_message = "Disk I/O operation failed";
    DiskManagerException disk_ex(disk_message);

    // 验证异常消息
    EXPECT_STREQ(disk_ex.what(), disk_message.c_str());

    // 验证继承关系
    EXPECT_TRUE(dynamic_cast<Exception*>(&disk_ex) != nullptr);
    EXPECT_TRUE(dynamic_cast<std::runtime_error*>(&disk_ex) != nullptr);

    // 测试抛出和捕获
    try {
        throw DiskManagerException("Disk space insufficient");
    } catch (const DiskManagerException& e) {
        EXPECT_STREQ(e.what(), "Disk space insufficient");
    } catch (...) {
        FAIL() << "Expected DiskManagerException to be caught";
    }
}

/**
 * @brief 测试LockTimeoutException
 */
TEST_F(ExceptionTest, LockTimeoutException) {
    // 测试构造函数
    std::string lock_message = "Lock acquisition timeout";
    LockTimeoutException lock_ex(lock_message);

    // 验证异常消息
    EXPECT_STREQ(lock_ex.what(), lock_message.c_str());

    // 验证继承关系
    EXPECT_TRUE(dynamic_cast<Exception*>(&lock_ex) != nullptr);
    EXPECT_TRUE(dynamic_cast<std::runtime_error*>(&lock_ex) != nullptr);

    // 测试抛出和捕获
    try {
        throw LockTimeoutException("Deadlock detected");
    } catch (const LockTimeoutException& e) {
        EXPECT_STREQ(e.what(), "Deadlock detected");
    } catch (...) {
        FAIL() << "Expected LockTimeoutException to be caught";
    }
}

/**
 * @brief 测试NotImplementedException
 */
TEST_F(ExceptionTest, NotImplementedException) {
    // 测试构造函数
    std::string feature_message = "Feature not yet implemented";
    NotImplementedException feature_ex(feature_message);

    // 验证异常消息
    EXPECT_STREQ(feature_ex.what(), feature_message.c_str());

    // 验证继承关系
    EXPECT_TRUE(dynamic_cast<Exception*>(&feature_ex) != nullptr);
    EXPECT_TRUE(dynamic_cast<std::runtime_error*>(&feature_ex) != nullptr);

    // 测试抛出和捕获
    try {
        throw NotImplementedException("Advanced SQL features");
    } catch (const NotImplementedException& e) {
        EXPECT_STREQ(e.what(), "Advanced SQL features");
    } catch (...) {
        FAIL() << "Expected NotImplementedException to be caught";
    }
}

/**
 * @brief 测试IllegalArgumentException
 */
TEST_F(ExceptionTest, IllegalArgumentException) {
    // 测试构造函数
    std::string arg_message = "Invalid argument provided";
    IllegalArgumentException arg_ex(arg_message);

    // 验证异常消息
    EXPECT_STREQ(arg_ex.what(), arg_message.c_str());

    // 验证继承关系
    EXPECT_TRUE(dynamic_cast<Exception*>(&arg_ex) != nullptr);
    EXPECT_TRUE(dynamic_cast<std::runtime_error*>(&arg_ex) != nullptr);

    // 测试抛出和捕获
    try {
        throw IllegalArgumentException("Negative value not allowed");
    } catch (const IllegalArgumentException& e) {
        EXPECT_STREQ(e.what(), "Negative value not allowed");
    } catch (...) {
        FAIL() << "Expected IllegalArgumentException to be caught";
    }
}

/**
 * @brief 测试异常的多态性
 */
TEST_F(ExceptionTest, Polymorphism) {
    // 创建不同类型的异常
    std::vector<std::unique_ptr<Exception>> exceptions;
    exceptions.push_back(std::make_unique<IOException>("I/O error"));
    exceptions.push_back(std::make_unique<BufferPoolException>("Buffer error"));
    exceptions.push_back(std::make_unique<PageException>("Page error"));
    exceptions.push_back(std::make_unique<DiskManagerException>("Disk error"));
    exceptions.push_back(std::make_unique<LockTimeoutException>("Lock timeout"));
    exceptions.push_back(std::make_unique<NotImplementedException>("Not implemented"));
    exceptions.push_back(std::make_unique<IllegalArgumentException>("Invalid argument"));

    // 使用基类指针测试多态性
    for (const auto& ex : exceptions) {
        // 验证都是Exception的子类
        EXPECT_TRUE(dynamic_cast<const Exception*>(ex.get()) != nullptr);

        // 验证消息不为空
        EXPECT_FALSE(std::string(ex->what()).empty());
    }
}

/**
 * @brief 测试异常的拷贝和赋值
 */
TEST_F(ExceptionTest, CopyAndAssignment) {
    // 测试拷贝构造函数
    IOException original("Original I/O error");
    IOException copy(original);

    EXPECT_STREQ(copy.what(), original.what());

    // 测试赋值操作符（如果支持的话）
    // 注意：std::runtime_error不支持拷贝赋值，但我们可以测试引用
    const IOException& ref = original;
    EXPECT_STREQ(ref.what(), "Original I/O error");
}

/**
 * @brief 测试异常层次结构
 */
TEST_F(ExceptionTest, ExceptionHierarchy) {
    // 测试异常类型识别
    try {
        throw IOException("Test I/O exception");
    } catch (const Exception& e) {
        // 应该能够用基类捕获
        EXPECT_STREQ(e.what(), "Test I/O exception");
        EXPECT_TRUE(typeid(e) == typeid(IOException) ||
                   typeid(e) == typeid(Exception));
    }

    try {
        throw BufferPoolException("Test buffer exception");
    } catch (const Exception& e) {
        EXPECT_STREQ(e.what(), "Test buffer exception");
    }

    try {
        throw NotImplementedException("Test feature exception");
    } catch (const Exception& e) {
        EXPECT_STREQ(e.what(), "Test feature exception");
    }
}

/**
 * @brief 测试异常消息的特殊字符处理
 */
TEST_F(ExceptionTest, SpecialCharactersInMessages) {
    // 测试包含特殊字符的消息
    std::string special_message = "Error with special chars: \n\t\r\"\\";
    Exception special_ex(special_message);

    EXPECT_STREQ(special_ex.what(), special_message.c_str());

    // 测试Unicode字符
    std::string unicode_message = "错误消息：数据库连接失败";
    Exception unicode_ex(unicode_message);

    EXPECT_STREQ(unicode_ex.what(), unicode_message.c_str());
}

/**
 * @brief 测试异常的内存管理
 */
TEST_F(ExceptionTest, MemoryManagement) {
    // 测试异常对象的动态分配
    Exception* dynamic_ex = new IOException("Dynamic exception");

    EXPECT_STREQ(dynamic_ex->what(), "Dynamic exception");

    // 测试智能指针
    std::unique_ptr<Exception> smart_ex = std::make_unique<PageException>("Smart pointer exception");
    EXPECT_STREQ(smart_ex->what(), "Smart pointer exception");

    // 清理动态分配的对象
    delete dynamic_ex;
}

/**
 * @brief 测试异常的抛出位置信息（如果可用）
 */
TEST_F(ExceptionTest, ExceptionLocation) {
    // 注意：标准C++异常不提供位置信息
    // 这里主要测试异常的基本功能
    try {
        throw DiskManagerException("Disk operation failed at line XYZ");
    } catch (const DiskManagerException& e) {
        EXPECT_THAT(std::string(e.what()), ::testing::HasSubstr("Disk operation failed"));
    }
}

/**
 * @brief 测试连续抛出多个异常
 */
TEST_F(ExceptionTest, MultipleExceptions) {
    // 测试在循环中抛出异常
    for (int i = 0; i < 5; ++i) {
        try {
            switch (i % 3) {
                case 0:
                    throw IOException("I/O error " + std::to_string(i));
                case 1:
                    throw BufferPoolException("Buffer error " + std::to_string(i));
                case 2:
                    throw IllegalArgumentException("Argument error " + std::to_string(i));
            }
        } catch (const IOException& e) {
            EXPECT_THAT(std::string(e.what()), ::testing::HasSubstr("I/O error"));
        } catch (const BufferPoolException& e) {
            EXPECT_THAT(std::string(e.what()), ::testing::HasSubstr("Buffer error"));
        } catch (const IllegalArgumentException& e) {
            EXPECT_THAT(std::string(e.what()), ::testing::HasSubstr("Argument error"));
        }
    }
}

/**
 * @brief 测试异常的嵌套抛出
 */
TEST_F(ExceptionTest, NestedExceptions) {
    // 测试在catch块中重新抛出异常
    try {
        try {
            throw IOException("Inner I/O error");
        } catch (const IOException& inner) {
            // 在内部catch中抛出新异常
            throw BufferPoolException("Caused by: " + std::string(inner.what()));
        }
    } catch (const BufferPoolException& outer) {
        EXPECT_THAT(std::string(outer.what()), ::testing::HasSubstr("Caused by"));
        EXPECT_THAT(std::string(outer.what()), ::testing::HasSubstr("Inner I/O error"));
    }
}

/**
 * @brief 测试异常的性能表现
 */
TEST_F(ExceptionTest, Performance) {
    // 测试异常抛出的性能（不抛出，只是创建）
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 10000; ++i) {
        Exception ex("Performance test message " + std::to_string(i));
        // 不抛出，只是创建和销毁
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // 验证创建10000个异常对象的时间在合理范围内（通常应该小于1秒）
    EXPECT_LT(duration.count(), 1000);  // 1秒
}
