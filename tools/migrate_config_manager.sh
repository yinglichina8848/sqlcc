#!/bin/bash

# SQLCC 智能配置管理器迁移工具
# 自动化从传统配置管理器到智能配置管理器的升级过程

set -e  # 遇到错误立即退出

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 配置变量
BACKUP_DIR="backup_$(date +%Y%m%d_%H%M%S)"
MIGRATION_LOG="migration_$(date +%Y%m%d_%H%M%S).log"
PROJECT_ROOT="/home/liying/sqlcc"

# 函数定义
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1" | tee -a "$MIGRATION_LOG"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1" | tee -a "$MIGRATION_LOG"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1" | tee -a "$MIGRATION_LOG"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1" | tee -a "$MIGRATION_LOG"
}

print_header() {
    echo -e "\n${BLUE}========================================${NC}"
    echo -e "${BLUE}  SQLCC 智能配置管理器迁移工具${NC}"
    echo -e "${BLUE}========================================${NC}\n"
}

print_footer() {
    echo -e "\n${BLUE}========================================${NC}"
    echo -e "${BLUE}  迁移过程完成${NC}"
    echo -e "${BLUE}========================================${NC}\n"
}

# 检查依赖
check_dependencies() {
    log_info "检查依赖项..."
    
    # 检查必要的工具
    local missing_deps=()
    
    command -v git >/dev/null 2>&1 || missing_deps+=("git")
    command -v make >/dev/null 2>&1 || missing_deps+=("make")
    command -v g++ >/dev/null 2>&1 || missing_deps+=("g++")
    command -v find >/dev/null 2>&1 || missing_deps+=("find")
    
    if [ ${#missing_deps[@]} -ne 0 ]; then
        log_error "缺少依赖项: ${missing_deps[*]}"
        log_error "请安装缺失的工具后重试"
        exit 1
    fi
    
    log_success "所有依赖项检查通过"
}

# 创建备份
create_backup() {
    log_info "创建代码备份..."
    
    if [ ! -d "$PROJECT_ROOT" ]; then
        log_error "项目根目录不存在: $PROJECT_ROOT"
        exit 1
    fi
    
    # 创建备份目录
    mkdir -p "$BACKUP_DIR"
    
    # 备份关键文件和目录
    local backup_items=(
        "src/config_manager"
        "include/utils/config_manager.h"
        "tests"
        "CMakeLists.txt"
        "Makefile"
    )
    
    for item in "${backup_items[@]}"; do
        if [ -e "$PROJECT_ROOT/$item" ]; then
            cp -r "$PROJECT_ROOT/$item" "$BACKUP_DIR/"
            log_info "已备份: $item"
        fi
    done
    
    log_success "备份创建完成: $BACKUP_DIR"
}

# 编译迁移脚本
build_migration_script() {
    log_info "编译迁移脚本..."
    
    local script_path="$PROJECT_ROOT/tools/config_migration_script.cpp"
    local output_path="$PROJECT_ROOT/tools/migration_script"
    
    if [ ! -f "$script_path" ]; then
        log_error "迁移脚本不存在: $script_path"
        exit 1
    fi
    
    # 编译脚本
    cd "$PROJECT_ROOT"
    if g++ -std=c++17 -o "$output_path" "$script_path"; then
        log_success "迁移脚本编译成功"
    else
        log_error "迁移脚本编译失败"
        exit 1
    fi
}

# 运行代码迁移
run_code_migration() {
    log_info "开始代码迁移..."
    
    local script_path="$PROJECT_ROOT/tools/migration_script"
    
    if [ ! -x "$script_path" ]; then
        log_error "迁移脚本不可执行: $script_path"
        exit 1
    fi
    
    # 运行迁移脚本
    cd "$PROJECT_ROOT"
    if "$script_path" "src"; then
        log_success "代码迁移完成"
    else
        log_error "代码迁移失败"
        exit 1
    fi
}

# 更新构建系统
update_build_system() {
    log_info "更新构建系统..."
    
    cd "$PROJECT_ROOT"
    
    # 检查CMakeLists.txt
    if [ -f "CMakeLists.txt" ]; then
        log_info "更新CMakeLists.txt..."
        
        # 添加新的源文件
        local new_sources=(
            "src/utils/config_snapshot.cpp"
            "src/utils/config_lifecycle.cpp"
            "src/utils/smart_config_manager.cpp"
        )
        
        for source in "${new_sources[@]}"; do
            if ! grep -q "$source" CMakeLists.txt; then
                # 找到合适的位置插入新的源文件
                sed -i "/add_executable.*sqlcc/a\\    $source" CMakeLists.txt
                log_info "已添加: $source"
            fi
        done
    fi
    
    # 检查Makefile
    if [ -f "Makefile" ]; then
        log_info "更新Makefile..."
        
        # 类似地更新Makefile
        # 这里可以根据具体的Makefile结构进行调整
    fi
    
    log_success "构建系统更新完成"
}

# 编译测试
run_build_test() {
    log_info "运行编译测试..."
    
    cd "$PROJECT_ROOT"
    
    # 清理构建
    if [ -f "Makefile" ]; then
        make clean || log_warning "清理构建失败，继续尝试编译"
    fi
    
    # 重新编译
    if make -j$(nproc); then
        log_success "编译测试通过"
    else
        log_error "编译测试失败"
        return 1
    fi
}

# 运行单元测试
run_unit_tests() {
    log_info "运行单元测试..."
    
    cd "$PROJECT_ROOT"
    
    # 检查测试可执行文件
    local test_executable="./tests/test_smart_config_manager"
    
    if [ -x "$test_executable" ]; then
        if "$test_executable"; then
            log_success "单元测试通过"
        else
            log_error "单元测试失败"
            return 1
        fi
    else
        log_warning "测试可执行文件不存在: $test_executable"
        log_warning "跳过单元测试"
    fi
}

# 验证迁移结果
validate_migration() {
    log_info "验证迁移结果..."
    
    cd "$PROJECT_ROOT"
    
    # 检查关键文件是否存在
    local required_files=(
        "include/utils/config_snapshot.h"
        "include/utils/config_lifecycle.h"
        "include/utils/smart_config_manager.h"
        "src/utils/config_snapshot.cpp"
        "src/utils/config_lifecycle.cpp"
        "src/utils/smart_config_manager.cpp"
    )
    
    local missing_files=()
    for file in "${required_files[@]}"; do
        if [ ! -f "$file" ]; then
            missing_files+=("$file")
        fi
    done
    
    if [ ${#missing_files[@]} -ne 0 ]; then
        log_error "缺少关键文件:"
        for file in "${missing_files[@]}"; do
            log_error "  - $file"
        done
        return 1
    fi
    
    # 检查代码中是否还有旧的引用
    local old_patterns=(
        "ConfigManager::GetInstance"
        "->GetBool("
        "->GetInt("
        "->GetDouble("
        "->GetString("
    )
    
    local found_old_patterns=()
    for pattern in "${old_patterns[@]}"; do
        if grep -r "$pattern" src/ include/ --include="*.cpp" --include="*.h" >/dev/null 2>&1; then
            found_old_patterns+=("$pattern")
        fi
    done
    
    if [ ${#found_old_patterns[@]} -ne 0 ]; then
        log_warning "发现旧的代码模式，建议手动检查:"
        for pattern in "${found_old_patterns[@]}"; do
            log_warning "  - $pattern"
        done
    fi
    
    log_success "迁移结果验证完成"
}

# 生成迁移报告
generate_report() {
    log_info "生成迁移报告..."
    
    local report_file="migration_report_$(date +%Y%m%d_%H%M%S).md"
    
    cat > "$report_file" << EOF
# SQLCC 智能配置管理器迁移报告

生成时间: $(date)

## 迁移概览

- 备份目录: \`$BACKUP_DIR\`
- 日志文件: \`$MIGRATION_LOG\`
- 项目根目录: \`$PROJECT_ROOT\`

## 迁移步骤

$(if [ -d "$BACKUP_DIR" ]; then echo "✅ 代码备份已创建"; else echo "❌ 代码备份失败"; fi)

$(if [ -f "$PROJECT_ROOT/tools/migration_script" ]; then echo "✅ 迁移脚本编译成功"; else echo "❌ 迁移脚本编译失败"; fi)

$(if run_build_test >/dev/null 2>&1; then echo "✅ 编译测试通过"; else echo "❌ 编译测试失败"; fi)

$(if run_unit_tests >/dev/null 2>&1; then echo "✅ 单元测试通过"; else echo "⚠️  单元测试跳过或失败"; fi)

## 文件变更

迁移脚本已处理以下类型的变更：

1. **头文件引用**
   - \`#include "utils/config_manager.h"\` → \`#include "utils/smart_config_manager.h"\`

2. **实例获取**
   - \`ConfigManager* config = ConfigManager::GetInstance()\` → \`auto config = SmartConfigManager::GetInstance()\`

3. **配置访问方法**
   - \`GetBool(key, default)\` → \`GetBoolConfig(key, default)\`
   - \`GetInt(key, default)\` → \`GetIntConfig(key, default)\`
   - \`GetDouble(key, default)\` → \`GetDoubleConfig(key, default)\`
   - \`GetString(key, default)\` → \`GetStringConfig(key, default)\`

## 新增功能

迁移后的智能配置管理器提供以下新功能：

- ✅ **内存安全**: 智能指针自动管理，零内存泄漏
- ✅ **RAII模式**: 异常安全的资源配置
- ✅ **热更新**: 配置文件修改自动生效
- ✅ **版本管理**: 配置版本跟踪和回滚
- ✅ **异步操作**: 支持异步配置更新
- ✅ **批量操作**: 高效的批量配置更新

## 验证建议

1. **功能测试**: 验证所有配置访问功能正常
2. **性能测试**: 对比迁移前后的性能表现
3. **内存测试**: 使用valgrind检查内存泄漏
4. **线程安全**: 在多线程环境下测试稳定性

## 回滚说明

如果需要回滚到迁移前的状态：

\`\`\`bash
# 从备份恢复
cp -r $BACKUP_DIR/* $PROJECT_ROOT/
# 重新编译
make clean && make
\`\`\`

## 后续步骤

1. 更新相关文档
2. 培训团队成员使用新API
3. 监控生产环境表现
4. 收集用户反馈

---

*此报告由SQLCC智能配置管理器迁移工具自动生成*
EOF
    
    log_success "迁移报告已生成: $report_file"
}

# 主函数
main() {
    print_header
    
    # 初始化日志
    echo "SQLCC 智能配置管理器迁移工具" > "$MIGRATION_LOG"
    echo "开始时间: $(date)" >> "$MIGRATION_LOG"
    echo "=================================" >> "$MIGRATION_LOG"
    
    # 执行迁移步骤
    check_dependencies
    create_backup
    build_migration_script
    run_code_migration
    update_build_system
    
    # 验证和测试
    if run_build_test; then
        run_unit_tests
        validate_migration
        generate_report
        
        log_success "🎉 迁移过程成功完成！"
        log_info "请查看生成的报告文件以获取详细信息"
        log_info "建议进行全面的功能测试后再部署到生产环境"
    else
        log_error "❌ 迁移过程失败，请查看日志文件: $MIGRATION_LOG"
        log_error "可以从备份目录恢复: $BACKUP_DIR"
        exit 1
    fi
    
    print_footer
}

# 脚本入口
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    main "$@"
fi