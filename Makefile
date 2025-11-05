# SQLCC Makefile
# 提供常用的构建和文档生成命令

.PHONY: all build test docs clean help

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

# 生成文档
docs:
	@echo "生成Doxygen文档..."
	@./scripts/generate_docs.sh
	@echo "文档生成完成!"

# 清理构建文件
clean:
	@echo "清理构建文件..."
	@rm -rf build
	@echo "清理完成!"

# 清理所有文件（包括文档）
clean-all: clean
	@echo "清理文档文件..."
	@rm -rf docs/doxygen
	@echo "全部清理完成!"

# 安装依赖（Ubuntu/Debian）
install-deps:
	@echo "安装依赖..."
	@sudo apt update
	@sudo apt install -y cmake build-essential doxygen
	@echo "依赖安装完成!"

# 显示帮助信息
help:
	@echo "SQLCC Makefile 命令:"
	@echo "  all        - 构建项目 (默认)"
	@echo "  build      - 构建项目"
	@echo "  test       - 运行单元测试"
	@echo "  docs       - 生成Doxygen文档"
	@echo "  clean      - 清理构建文件"
	@echo "  clean-all  - 清理所有文件（包括文档）"
	@echo "  install-deps- 安装依赖（Ubuntu/Debian）"
	@echo "  help       - 显示此帮助信息"