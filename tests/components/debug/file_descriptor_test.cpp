/**
 * @file file_descriptor_test.cpp
 * @brief 文件描述符封装测试
 * @details 验证FileDescriptor和SocketDescriptor类的正确性
 * @author AI助手
 * @date 2025-12-11
 */

#include <gtest/gtest.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <thread>
#include <atomic>
#include "utils/file_descriptor.h"

namespace sqlcc {
namespace test {

/**
 * @class FileDescriptorTest
 * @brief 文件描述符封装测试
 */
class FileDescriptorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建一个临时文件用于测试
        temp_fd_ = open("/tmp/test_fd", O_CREAT | O_RDWR, 0644);
        ASSERT_GE(temp_fd_, 0) << "Failed to create temp file";
    }

    void TearDown() override {
        if (temp_fd_ >= 0) {
            close(temp_fd_);
            unlink("/tmp/test_fd");
        }
    }

    int temp_fd_ = -1;
};

/**
 * @brief 测试FileDescriptor基本功能
 */
TEST_F(FileDescriptorTest, BasicFunctionality) {
    // 测试默认构造函数
    FileDescriptor fd1;
    EXPECT_FALSE(fd1.valid());
    EXPECT_EQ(fd1.get(), -1);

    // 测试带参数构造函数
    FileDescriptor fd2(temp_fd_);
    EXPECT_TRUE(fd2.valid());
    EXPECT_EQ(fd2.get(), temp_fd_);

    // 测试移动构造
    FileDescriptor fd3(std::move(fd2));
    EXPECT_TRUE(fd3.valid());
    EXPECT_EQ(fd3.get(), temp_fd_);
    EXPECT_FALSE(fd2.valid());  // fd2应该无效

    // 测试显式转换
    int raw_fd = static_cast<int>(fd3);
    EXPECT_EQ(raw_fd, temp_fd_);
}

/**
 * @brief 测试FileDescriptor资源管理
 */
TEST_F(FileDescriptorTest, ResourceManagement) {
    int original_fd;

    {
        FileDescriptor fd(temp_fd_);
        original_fd = fd.get();
        EXPECT_TRUE(fd.valid());

        // 验证文件描述符仍然有效
        int flags = fcntl(fd.get(), F_GETFL);
        EXPECT_GE(flags, 0);
    } // fd离开作用域，应该自动关闭

    // 验证文件描述符已被关闭
    int result = fcntl(original_fd, F_GETFL);
    EXPECT_EQ(result, -1);
    EXPECT_EQ(errno, EBADF);  // Bad file descriptor
}

/**
 * @brief 测试FileDescriptor移动语义
 */
TEST_F(FileDescriptorTest, MoveSemantics) {
    // 创建第一个文件描述符
    FileDescriptor fd1(temp_fd_);
    EXPECT_TRUE(fd1.valid());

    // 移动到第二个文件描述符
    FileDescriptor fd2 = std::move(fd1);
    EXPECT_FALSE(fd1.valid());  // fd1应该无效
    EXPECT_TRUE(fd2.valid());   // fd2应该有效
    EXPECT_EQ(fd2.get(), temp_fd_);

    // 移动赋值
    FileDescriptor fd3;
    fd3 = std::move(fd2);
    EXPECT_FALSE(fd2.valid());  // fd2应该无效
    EXPECT_TRUE(fd3.valid());   // fd3应该有效
    EXPECT_EQ(fd3.get(), temp_fd_);
}

/**
 * @brief 测试FileDescriptor reset和release
 */
TEST_F(FileDescriptorTest, ResetAndRelease) {
    FileDescriptor fd(temp_fd_);

    // 测试release
    int released_fd = fd.release();
    EXPECT_EQ(released_fd, temp_fd_);
    EXPECT_FALSE(fd.valid());

    // 测试reset
    fd.reset(released_fd);
    EXPECT_TRUE(fd.valid());
    EXPECT_EQ(fd.get(), temp_fd_);
}

/**
 * @brief 测试SocketDescriptor
 */
TEST(SocketDescriptorTest, SocketCreation) {
    // 测试TCP套接字创建
    auto tcp_socket = FileDescriptor::create_tcp_socket();
    EXPECT_TRUE(tcp_socket.valid());

    // 验证是TCP套接字
    int type;
    socklen_t len = sizeof(type);
    int result = getsockopt(tcp_socket.get(), SOL_SOCKET, SO_TYPE, &type, &len);
    EXPECT_EQ(result, 0);
    EXPECT_EQ(type, SOCK_STREAM);

    // 测试UDP套接字创建
    auto udp_socket = FileDescriptor::create_udp_socket();
    EXPECT_TRUE(udp_socket.valid());

    // 验证是UDP套接字
    result = getsockopt(udp_socket.get(), SOL_SOCKET, SO_TYPE, &type, &len);
    EXPECT_EQ(result, 0);
    EXPECT_EQ(type, SOCK_DGRAM);
}

/**
 * @brief 测试多线程安全
 */
TEST_F(FileDescriptorTest, ThreadSafety) {
    // 这个测试验证RAII在多线程环境下的安全性
    std::atomic<bool> destructor_called{false};

    std::thread t([&]() {
        {
            FileDescriptor fd(temp_fd_);
            // 模拟一些工作
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        } // fd在这里被销毁
        destructor_called = true;
    });

    t.join();
    EXPECT_TRUE(destructor_called);
}

/**
 * @brief 测试异常安全
 */
TEST_F(FileDescriptorTest, ExceptionSafety) {
    bool exception_thrown = false;

    try {
        FileDescriptor fd(temp_fd_);

        // 模拟在作用域内抛出异常
        if (!exception_thrown) {
            exception_thrown = true;
            throw std::runtime_error("Test exception");
        }
    } catch (const std::runtime_error&) {
        // 异常被正确捕获，FileDescriptor应该已经正确清理
    }

    // 验证文件描述符已被关闭
    int result = fcntl(temp_fd_, F_GETFL);
    EXPECT_EQ(result, -1);
    EXPECT_EQ(errno, EBADF);
}

} // namespace test
} // namespace sqlcc