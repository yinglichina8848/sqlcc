#pragma once

#include <gmock/gmock.h>
#include "src/src/core/config_manager.h"

namespace sqlcc {
namespace test {

class MockConfigManager : public ConfigManager {
public:
    MOCK_METHOD(std::string, GetDatabasePath, (), (const, override));
    MOCK_METHOD(int, GetBufferSize, (), (const, override));
    MOCK_METHOD(bool, GetLoggingEnabled, (), (const, override));
    MOCK_METHOD(std::string, GetLogLevel, (), (const, override));
};

} // namespace test
} // namespace sqlcc