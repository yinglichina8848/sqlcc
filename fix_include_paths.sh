#!/bin/bash
# SQLCC Include路径自动修复脚本
# Generated automatically

# 修复 src/logger/logger.cpp:10
sed -i '10s|#include "../include/utils/logger.h"|#include "utils/logger.h"|' "src/logger/logger.cpp"

# 修复 src/utils/config_snapshot.cpp:10
sed -i '10s|#include "../include/utils/config_snapshot.h"|#include "utils/config_snapshot.h"|' "src/utils/config_snapshot.cpp"

# 修复 src/utils/config_manager.cpp:1
sed -i '1s|#include "../../include/utils/config_manager.h"|#include "utils/config_manager.h"|' "src/utils/config_manager.cpp"

# 修复 src/utils/smart_config_manager.cpp:10
sed -i '10s|#include "../include/utils/smart_config_manager.h"|#include "utils/smart_config_manager.h"|' "src/utils/smart_config_manager.cpp"

