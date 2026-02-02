#include <gtest/gtest.h>
#include <string>
#include <cstdio>
#include <fstream>

#include "src/utils/file_descriptor.h"
#include "src/utils/version.h"

namespace sqlcc {
namespace test {

// ==================== FileDescriptor Tests ====================

class FileDescriptorTest : public ::testing::Test {
protected:
    void SetUp() override {
    }

    void TearDown() override {
    }
};

// 测试默认构造
TEST_F(FileDescriptorTest, DefaultConstructor) {
    FileDescriptor fd;
    EXPECT_FALSE(fd.valid());
    EXPECT_EQ(fd.get(), -1);
}

// 测试有效文件描述符
TEST_F(FileDescriptorTest, ValidFileDescriptor) {
    std::string test_file = "/tmp/sqlcc_fd_test_" + std::to_string(__LINE__) + ".txt";
    
    // 创建临时文件
    std::ofstream ofs(test_file);
    ofs << "test";
    ofs.close();
    
    // 打开文件获取文件描述符
    FILE* file = fopen(test_file.c_str(), "r");
    ASSERT_NE(file, nullptr);
    
    FileDescriptor fd(fileno(file));
    EXPECT_TRUE(fd.valid());
    EXPECT_GE(fd.get(), 0);
    
    fclose(file);
    std::remove(test_file.c_str());
}

// 测试 move 构造
TEST_F(FileDescriptorTest, MoveConstructor) {
    std::string test_file = "/tmp/sqlcc_fd_test_" + std::to_string(__LINE__) + ".txt";
    
    std::ofstream ofs(test_file);
    ofs << "test";
    ofs.close();
    
    FILE* file = fopen(test_file.c_str(), "r");
    FileDescriptor fd1(fileno(file));
    
    FileDescriptor fd2(std::move(fd1));
    
    EXPECT_FALSE(fd1.valid());
    EXPECT_TRUE(fd2.valid());
    
    fclose(file);
    std::remove(test_file.c_str());
}

// 测试 move 赋值
TEST_F(FileDescriptorTest, MoveAssignment) {
    std::string test_file1 = "/tmp/sqlcc_fd_test1_" + std::to_string(__LINE__) + ".txt";
    std::string test_file2 = "/tmp/sqlcc_fd_test2_" + std::to_string(__LINE__) + ".txt";
    
    std::ofstream ofs1(test_file1);
    ofs1 << "test1";
    ofs1.close();
    
    std::ofstream ofs2(test_file2);
    ofs2 << "test2";
    ofs2.close();
    
    FILE* file1 = fopen(test_file1.c_str(), "r");
    FILE* file2 = fopen(test_file2.c_str(), "r");
    
    FileDescriptor fd1(fileno(file1));
    FileDescriptor fd2(fileno(file2));
    
    fd1 = std::move(fd2);
    
    EXPECT_TRUE(fd1.valid());
    EXPECT_FALSE(fd2.valid());
    
    fclose(file1);
    fclose(file2);
    std::remove(test_file1.c_str());
    std::remove(test_file2.c_str());
}

// 测试 release 方法
TEST_F(FileDescriptorTest, Release) {
    std::string test_file = "/tmp/sqlcc_fd_test_" + std::to_string(__LINE__) + ".txt";
    
    std::ofstream ofs(test_file);
    ofs << "test";
    ofs.close();
    
    FILE* file = fopen(test_file.c_str(), "r");
    FileDescriptor fd(fileno(file));
    int original_fd = fd.get();
    
    // release() 返回原始 fd 值
    int released_fd = fd.release();
    
    EXPECT_EQ(released_fd, original_fd);  // released_fd 是原始值
    EXPECT_FALSE(fd.valid());  // fd 现在无效
    EXPECT_EQ(fd.get(), -1);  // fd 内部值变为 -1
    
    fclose(file);
    std::remove(test_file.c_str());
}

// 测试 reset 方法
TEST_F(FileDescriptorTest, Reset) {
    std::string test_file = "/tmp/sqlcc_fd_test_" + std::to_string(__LINE__) + ".txt";
    
    std::ofstream ofs(test_file);
    ofs << "test";
    ofs.close();
    
    FILE* file = fopen(test_file.c_str(), "r");
    FileDescriptor fd;
    
    fd.reset(fileno(file));
    EXPECT_TRUE(fd.valid());
    
    fd.reset(-1);
    EXPECT_FALSE(fd.valid());
    
    std::remove(test_file.c_str());
}

// 测试显式 bool 转换
TEST_F(FileDescriptorTest, ExplicitBoolConversion) {
    FileDescriptor fd;
    EXPECT_FALSE(static_cast<bool>(fd));
    
    std::string test_file = "/tmp/sqlcc_fd_test_" + std::to_string(__LINE__) + ".txt";
    std::ofstream ofs(test_file);
    ofs << "test";
    ofs.close();
    
    FILE* file = fopen(test_file.c_str(), "r");
    FileDescriptor fd2(fileno(file));
    EXPECT_TRUE(static_cast<bool>(fd2));
    
    fclose(file);
    std::remove(test_file.c_str());
}

// ==================== Version Tests ====================

class VersionTest : public ::testing::Test {
protected:
    void SetUp() override {
    }

    void TearDown() override {
    }
};

// 测试 get_version_string
TEST_F(VersionTest, GetVersionString) {
    std::string version = utils::get_version_string();
    EXPECT_FALSE(version.empty());
    EXPECT_TRUE(version.find("SQLCC") != std::string::npos);
}

// 测试 get_version
TEST_F(VersionTest, GetVersion) {
    std::string version = utils::get_version();
    EXPECT_FALSE(version.empty());
    // 格式应为 x.y.z
    EXPECT_TRUE(version.find(".") != std::string::npos);
}

// 测试 get_version_major
TEST_F(VersionTest, GetVersionMajor) {
    int major = utils::get_version_major();
    EXPECT_GE(major, 1);
}

// 测试 get_version_minor
TEST_F(VersionTest, GetVersionMinor) {
    int minor = utils::get_version_minor();
    EXPECT_GE(minor, 0);
}

// 测试 get_version_patch
TEST_F(VersionTest, GetVersionPatch) {
    int patch = utils::get_version_patch();
    EXPECT_GE(patch, 0);
}

// 测试版本宏
TEST_F(VersionTest, VersionMacros) {
    EXPECT_GE(SQLCC_VERSION_MAJOR, 1);
    EXPECT_GE(SQLCC_VERSION_MINOR, 0);
    EXPECT_GE(SQLCC_VERSION_PATCH, 0);
    EXPECT_FALSE(std::string(SQLCC_VERSION).empty());
    EXPECT_FALSE(std::string(SQLCC_VERSION_STRING).empty());
}

} // namespace test
} // namespace sqlcc
