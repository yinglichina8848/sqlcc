#!/bin/bash
#
# SQLCC 统一测试脚本
# 运行所有Level1和Level2测试并生成统一报告
#

set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${PROJECT_ROOT}"

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

echo "========================================="
echo "  SQLCC 统一测试脚本"
echo "========================================="
echo "开始时间: $(date)"
echo ""

# 配置输出目录
OUTPUT_DIR="${PROJECT_ROOT}/tests/test_output"
TIMESTAMP=$(date +%Y%m%d_%H%M%S)
BUILD_DIR="${OUTPUT_DIR}/build"
LOG_DIR="${OUTPUT_DIR}/logs"
RESULTS_DIR="${OUTPUT_DIR}/results"

# 创建输出目录
log_info "创建输出目录..."
mkdir -p "${BUILD_DIR}"
mkdir -p "${LOG_DIR}/build"
mkdir -p "${LOG_DIR}/test"
mkdir -p "${RESULTS_DIR}"

# 清理旧的构建
log_info "清理旧的构建输出..."
bazel clean --expunge 2>/dev/null || true

# 运行Level1测试
log_info "运行Level1基础组件测试..."
if bazel test //tests/level1_foundation/... \
    --test_output=errors \
    --build_tag_filters=-manual \
    2>&1 | tee "${LOG_DIR}/test/level1_test_${TIMESTAMP}.log"; then
    log_success "Level1测试完成"
    LEVEL1_STATUS="✅ 通过"
else
    log_warn "Level1测试部分失败"
    LEVEL1_STATUS="⚠️ 部分失败"
fi

# 运行Level2测试
log_info "运行Level2核心服务测试..."
if bazel test //tests/level2_core_services/... \
    --test_output=errors \
    --build_tag_filters=-manual \
    2>&1 | tee "${LOG_DIR}/test/level2_test_${TIMESTAMP}.log"; then
    log_success "Level2测试完成"
    LEVEL2_STATUS="✅ 通过"
else
    log_warn "Level2测试部分失败"
    LEVEL2_STATUS="⚠️ 部分失败"
fi

# 运行Level2存储引擎测试
log_info "运行Level2存储引擎测试..."
if bazel test //tests/level2_storage_engine/... \
    --test_output=errors \
    --build_tag_filters=-manual \
    2>&1 | tee "${LOG_DIR}/test/level2_storage_test_${TIMESTAMP}.log"; then
    log_success "Level2存储引擎测试完成"
    LEVEL2_STORAGE_STATUS="✅ 通过"
else
    log_warn "Level2存储引擎测试部分失败"
    LEVEL2_STORAGE_STATUS="⚠️ 部分失败"
fi

# 运行Level3测试
log_info "运行Level3事务管理测试..."
if bazel test //tests/level3_transaction_manager/... \
    --test_output=errors \
    --build_tag_filters=-manual \
    2>&1 | tee "${LOG_DIR}/test/level3_test_${TIMESTAMP}.log"; then
    log_success "Level3测试完成"
    LEVEL3_STATUS="✅ 通过"
else
    log_warn "Level3测试部分失败"
    LEVEL3_STATUS="⚠️ 部分失败"
fi

# 生成测试结果摘要
log_info "生成测试结果摘要..."

cat > "${RESULTS_DIR}/test_results_${TIMESTAMP}.json" << EOF
{
  "timestamp": "${TIMESTAMP}",
  "test_date": "$(date '+%Y-%m-%d %H:%M:%S')",
  "test_results": {
    "level1_foundation": {
      "status": "${LEVEL1_STATUS}",
      "log_file": "${LOG_DIR}/test/level1_test_${TIMESTAMP}.log"
    },
    "level2_core_services": {
      "status": "${LEVEL2_STATUS}",
      "log_file": "${LOG_DIR}/test/level2_test_${TIMESTAMP}.log"
    },
    "level2_storage_engine": {
      "status": "${LEVEL2_STORAGE_STATUS}",
      "log_file": "${LOG_DIR}/test/level2_storage_test_${TIMESTAMP}.log"
    },
    "level3_transaction_manager": {
      "status": "${LEVEL3_STATUS}",
      "log_file": "${LOG_DIR}/test/level3_test_${TIMESTAMP}.log"
    }
  }
}
EOF

log_success "测试结果摘要生成完成"

echo ""
echo "========================================="
echo "测试执行完成!"
echo "========================================="
echo ""
echo "测试结果:"
echo "  - Level1基础组件: ${LEVEL1_STATUS}"
echo "  - Level2核心服务: ${LEVEL2_STATUS}"
echo "  - Level2存储引擎: ${LEVEL2_STORAGE_STATUS}"
echo "  - Level3事务管理: ${LEVEL3_STATUS}"
echo ""
echo "日志位置:"
echo "  - Level1: ${LOG_DIR}/test/level1_test_${TIMESTAMP}.log"
echo "  - Level2: ${LOG_DIR}/test/level2_test_${TIMESTAMP}.log"
echo "  - Level2存储: ${LOG_DIR}/test/level2_storage_test_${TIMESTAMP}.log"
echo "  - Level3: ${LOG_DIR}/test/level3_test_${TIMESTAMP}.log"
echo ""
echo "结果文件:"
echo "  - ${RESULTS_DIR}/test_results_${TIMESTAMP}.json"
echo ""
echo "完成时间: $(date)"
echo "========================================="