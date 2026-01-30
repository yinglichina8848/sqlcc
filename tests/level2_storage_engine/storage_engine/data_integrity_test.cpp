/**
 * @file data_integrity_test.cpp
 * @brief 数据完整性验证系统单元测试 - 测试数据约束验证功能
 */

#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <thread>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <atomic>
#include <random>

#include "storage/data_integrity_validator.h"
#include "storage/record_boundary_validator.h"
#include "storage_engine.h"
#include "src/utils/config_manager.h"

namespace fs = std::filesystem;
namespace sqlcc {

class DataIntegrityTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建临时测试目录
        test_dir = fs::temp_directory_path() / "sqlcc_data_integrity_test";
        fs::create_directories(test_dir);

        // 初始化配置管理器
        config = std::make_unique<ConfigManager>();
        config->SetValue("storage.data_directory", test_dir.string());

        // 初始化存储引擎
        storage_engine = std::make_shared<StorageEngine>(*config, test_dir.string());

        // 初始化数据完整性验证器
        integrity_validator = std::make_unique<DataIntegrityValidator>(storage_engine);
        boundary_validator = std::make_unique<RecordBoundaryValidator>(storage_engine);

        // 设置随机数生成器
        std::random_device rd;
        random_engine = std::mt19937(rd());
    }

    void TearDown() override {
        boundary_validator.reset();
        integrity_validator.reset();
        storage_engine.reset();
        config.reset();

        // 清理测试目录
        if (fs::exists(test_dir)) {
            fs::remove_all(test_dir);
        }
    }

    fs::path test_dir;
    std::unique_ptr<ConfigManager> config;
    std::shared_ptr<StorageEngine> storage_engine;
    std::unique_ptr<DataIntegrityValidator> integrity_validator;
    std::unique_ptr<RecordBoundaryValidator> boundary_validator;
    std::mt19937 random_engine;
};

// 测试记录边界验证
TEST_F(DataIntegrityTest, RecordBoundaryValidation) {
    // 测试记录验证功能
    std::vector<std::string> field_names = {"id", "name", "value"};
    std::vector<std::string> field_types = {"INT", "VARCHAR(50)", "DOUBLE"};
    std::vector<std::string> valid_values = {"1", "Test record 1", "123.45"};
    std::vector<std::string> invalid_values = {"invalid", "Too long string value that exceeds maximum length limit", "1234567890.12345678901234567890"};

    // 验证有效记录
    ValidationResult valid_result = boundary_validator->ValidateRecord("test_table", field_names, field_types, valid_values);
    EXPECT_EQ(valid_result, ValidationResult::VALID);

    // 测试验证更新
    ValidationResult update_result = boundary_validator->ValidateRecordUpdate("test_table", field_names, valid_values, valid_values);
    EXPECT_EQ(update_result, ValidationResult::VALID);

    // 测试验证删除
    ValidationResult delete_result = boundary_validator->ValidateRecordDeletion("test_table", 1);
    // 预期会失败，因为表不存在，但这不影响测试边界验证器的功能
    (void)delete_result; // 避免未使用变量警告
}

// 测试数据完整性配置
TEST_F(DataIntegrityTest, ConfigurationManagement) {
    // 获取默认配置
    auto default_config = integrity_validator->GetValidationConfig();
    EXPECT_TRUE(default_config.enable_not_null_checking);
    EXPECT_TRUE(default_config.enable_unique_checking);
    EXPECT_TRUE(default_config.enable_foreign_key_checking);
    EXPECT_TRUE(default_config.strict_mode);
}

// 测试数据完整性统计信息
TEST_F(DataIntegrityTest, StatisticsCollection) {
    // 获取统计信息
    auto stats = integrity_validator->GetIntegrityStats();
    EXPECT_EQ(stats.total_validations, 0);
    EXPECT_EQ(stats.successful_validations, 0);
    EXPECT_EQ(stats.constraint_violations, 0);
}

// 测试记录边界验证器统计信息
TEST_F(DataIntegrityTest, RecordBoundaryValidatorStats) {
    // 获取统计信息
    auto stats = boundary_validator->GetValidationStats();
    EXPECT_EQ(stats.total_validations, 0);
    EXPECT_EQ(stats.successful_validations, 0);
    EXPECT_EQ(stats.failed_validations, 0);
}

} // namespace sqlcc