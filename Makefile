# SQLCC Makefile
# 提供常用的构建和文档生成命令

.PHONY: all build test docs clean help coverage

# 默认目标
all: build

# 构建项目
build:
	@echo "构建SQLCC项目..."
	@mkdir -p build
	@cd build && cmake .. && make
	@echo "构建完成!"

# 运行测试
test: build
	@echo "运行单元测试..."
	@cd build && ./bin/storage_engine_test
	@echo "测试完成!"

<<<<<<< Updated upstream
# 生成文档
=======
# 验证构建结果
verify: build
	@echo "验证构建结果..."
	@./scripts/verify_build.sh $(BUILD_DIR)
	@echo "验证完成!"

# 运行覆盖率测试并生成报告
coverage: BUILD_DIR := build-coverage
coverage: CMAKE_FLAGS := -DCMAKE_BUILD_TYPE=Coverage
coverage:
	@echo "构建带覆盖率支持的项目..."
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake $(CMAKE_FLAGS) ..
	@cd $(BUILD_DIR) && make -j$(shell nproc)
	@echo "运行覆盖率测试..."
	@cd $(BUILD_DIR) && ctest --output-on-failure
	@echo "生成覆盖率报告..."
	@mkdir -p docs/coverage
	@cd $(BUILD_DIR) && gcovr --root .. --html --html-details -o ../docs/coverage/index.html \
		--exclude ../tests/ --exclude ../build/ --merge-mode-functions=merge-use-line-0 \
		--object-directory . --gcov-object-directory .
	@echo "清理临时gcov文件..."
	@cd $(BUILD_DIR) && find . -name "*.gcov" -delete
	@echo "覆盖率报告生成完成! 报告位置: docs/coverage/index.html"

# 生成Doxygen文档
>>>>>>> Stashed changes
docs:
	@echo "生成Doxygen文档..."
	@./scripts/generate_docs.sh
	@echo "文档生成完成!"

<<<<<<< Updated upstream
# 代码覆盖率测试
coverage:
=======
# 代码覆盖率测试（备用方法）
coverage-alt:
>>>>>>> Stashed changes
	@echo "构建带覆盖率支持的项目..."
	@mkdir -p build
	@cd build && cmake -DENABLE_COVERAGE=ON .. && make
	@echo "运行测试并生成覆盖率数据..."
	@cd build && ./bin/storage_engine_test
	@echo "生成HTML覆盖率报告..."
	@mkdir -p docs/coverage
	@lcov --capture --directory build --output-file docs/coverage/coverage.info --ignore-errors mismatch,gcov,gcov
	@lcov --remove docs/coverage/coverage.info '/usr/*' '/opt/*' --output-file docs/coverage/coverage_filtered.info --ignore-errors unused
	@genhtml docs/coverage/coverage_filtered.info --output-directory docs/coverage --ignore-errors source
	@echo "覆盖率报告已生成到 docs/coverage/index.html"
	@echo "清理临时文件..."
	@cd build && lcov --zerocounters --directory .

# 清理构建文件
clean:
	@echo "清理构建文件..."
	@rm -rf build
	@echo "清理完成!"

<<<<<<< Updated upstream
# 清理所有文件（包括文档）
=======
# 清理所有文件（包括文档和gcov文件）
>>>>>>> Stashed changes
clean-all: clean
	@echo "清理文档文件..."
	@rm -rf docs/doxygen
	@rm -rf docs/coverage
<<<<<<< Updated upstream
	@echo "全部清理完成!"

=======
	@echo "清理主目录中的gcov文件..."
	@find . -maxdepth 1 -name "*.gcov" -delete
	@echo "全部清理完成!"

# 清理gcov文件
clean-gcov:
	@echo "清理gcov文件..."
	@find . -name "*.gcov" -delete
	@echo "gcov文件清理完成!"
>>>>>>> Stashed changes
# 安装依赖（Ubuntu/Debian）
install-deps:
	@echo "安装依赖..."
	@sudo apt update
	@sudo apt install -y cmake build-essential doxygen lcov
	@echo "依赖安装完成!"

# 显示帮助信息
help:
	@echo "SQLCC Makefile 命令:"
<<<<<<< Updated upstream
	@echo "  all        - 构建项目 (默认)"
	@echo "  build      - 构建项目"
	@echo "  test       - 运行单元测试"
	@echo "  docs       - 生成Doxygen文档"
	@echo "  coverage   - 运行代码覆盖率测试并生成HTML报告"
	@echo "  clean      - 清理构建文件"
	@echo "  clean-all  - 清理所有文件（包括文档）"
=======
	@echo "  all/release - 构建Release版本并验证 (默认)"
	@echo "  debug       - 构建Debug版本"
	@echo "  build       - 构建项目 (使用当前配置)"
	@echo "  test        - 运行所有单元测试"
	@echo "  verify      - 验证构建结果"
	@echo "  coverage    - 运行覆盖率测试并生成报告"
	@echo "  docs        - 生成Doxygen文档"
	@echo "  clean       - 清理构建文件"
	@echo "  clean-all   - 清理所有文件（包括文档）"
	@echo "  clean-gcov  - 清理所有gcov文件"
>>>>>>> Stashed changes
	@echo "  install-deps- 安装依赖（Ubuntu/Debian）"
	@echo "  help       - 显示此帮助信息"