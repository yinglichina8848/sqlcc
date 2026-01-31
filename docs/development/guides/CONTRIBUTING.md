# 贡献指南

## 欢迎

感谢您对SQLCC项目的兴趣！我们欢迎各种形式的贡献，包括但不限于：

- 🐛 报告bug
- 💡 提出新功能建议
- 📝 改进文档
- 🔧 提交代码修复
- 🎨 改进用户界面
- 📊 性能优化
- 🧪 添加测试用例

## 快速开始

### 1. 环境准备

在开始贡献之前，请确保您的开发环境满足以下要求：

```bash
# 克隆项目
git clone https://gitee.com/yinglichina/sqlcc.git
cd sqlcc

# 安装依赖（根据您的系统选择）
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install clang-20 clang++-20 llvm-20 bazel-8.5

# CentOS/RHEL
sudo yum install clang-20 clang++-20 llvm-20 bazel

# macOS (使用Homebrew)
brew install clang@20 llvm@20 bazel
```

### 2. 构建项目

```bash
# 使用Bazel构建
bazel build //...

# 运行测试
bazel test //...
bazel test //...
```

### 3. 开发工作流

```bash
# 创建功能分支
git checkout -b feature/your-feature-name

# 编写代码并提交
git add .
git commit -m "feat: add your feature description"

# 推送到远程分支
git push origin feature/your-feature-name

# 创建Pull Request
```

## 开发规范

### 代码风格

SQLCC遵循严格的编码规范，请参考[编码规范文档](docs/code/coding_standards.md)。

#### 关键要求：
- **智能指针优先**：95%+代码使用`std::unique_ptr`、`std::shared_ptr`
- **RAII模式**：所有资源使用RAII模式管理
- **异常安全**：确保强异常安全保证
- **命名规范**：类用PascalCase，函数用snake_case

#### 代码示例：

```cpp
// ✅ 正确示例
class BufferPoolManager {
private:
    std::unique_ptr<PageTable> page_table_;
    std::vector<std::shared_ptr<Page>> pages_;

public:
    std::shared_ptr<Page> get_page(page_id_t page_id) {
        // 实现逻辑
    }
};

// ❌ 错误示例
class BadExample {
private:
    PageTable* page_table_;  // 裸指针，不明确所有权
};
```

### 提交规范

我们使用[Conventional Commits](https://conventionalcommits.org/)规范：

```bash
# 格式：<type>(<scope>): <subject>
# 示例：
git commit -m "feat(storage): add LRU cache for buffer pool"
git commit -m "fix(parser): handle null pointer in AST generation"
git commit -m "docs(api): update storage engine interface documentation"
git commit -m "test(executor): add comprehensive JOIN test cases"
```

#### 提交类型：
- `feat`: 新功能
- `fix`: 修复bug
- `docs`: 文档更新
- `style`: 代码格式调整
- `refactor`: 代码重构
- `test`: 测试相关
- `chore`: 构建工具、配置等

### 测试要求

#### 单元测试
- **覆盖率要求**：核心功能≥85%行覆盖率
- **测试命名**：`TEST(TestSuite, TestCase_Scenario_ExpectedResult)`
- **边界测试**：包含正常、异常、边界条件测试

```cpp
TEST(BufferPoolTest, GetPage_ExistingPage_ReturnsValidPage) {
    auto bpm = std::make_unique<BufferPoolManager>();
    auto page = bpm->get_page(1);
    ASSERT_NE(page, nullptr);
    ASSERT_EQ(page->get_page_id(), 1);
}
```

#### 性能测试
- 使用Google Benchmark进行性能基准测试
- 确保性能不退化（设置阈值）
- 包含内存使用分析

### 文档要求

#### 代码注释
- **函数注释**：所有public函数必须有完整的Doxygen注释
- **类注释**：说明类的用途和使用方法
- **复杂逻辑**：关键算法和业务逻辑必须注释

```cpp
/**
 * @brief 缓冲池管理器
 *
 * 负责管理数据库页面的缓存，支持LRU替换策略和并发访问。
 * 实现了16分片架构以提高并发性能。
 */
class BufferPoolManager {
public:
    /**
     * @brief 获取页面
     * @param page_id 页面ID
     * @return 页面智能指针，失败时返回nullptr
     * @throws BufferPoolException 当缓冲池满且无法置换时抛出
     */
    std::shared_ptr<Page> get_page(page_id_t page_id);
};
```

#### 文档更新
- 修改功能时同步更新相关文档
- 新功能必须提供使用示例
- API变更必须更新接口文档

## Pull Request流程

### 1. 创建PR

在提交Pull Request之前，请确保：

- [ ] 代码通过所有测试
- [ ] 代码遵循编码规范
- [ ] 提交信息符合规范
- [ ] 相关文档已更新
- [ ] 性能测试通过

### 2. PR描述

PR描述应该包含：

```markdown
## 描述
简要说明这次修改的目的和内容

## 类型
- [ ] Bug修复
- [ ] 新功能
- [ ] 文档更新
- [ ] 性能优化
- [ ] 重构

## 测试
- [ ] 单元测试通过
- [ ] 集成测试通过
- [ ] 性能测试通过

## 相关Issue
Fixes #123
```

### 3. 代码审查

PR提交后将进入代码审查流程：

1. **自动化检查**：CI/CD流水线运行测试和检查
2. **初步审查**：维护者检查基本要求
3. **深入审查**：领域专家审查代码质量和设计
4. **合并**：审查通过后合并到主分支

### 4. 审查标准

- **功能正确性**：代码实现需求，边界条件处理正确
- **代码质量**：遵循编码规范，结构清晰，可维护性好
- **测试充分性**：测试覆盖充分，包含异常情况
- **文档完整性**：相关文档更新完整
- **性能影响**：性能测试通过，无明显性能退化

## 问题报告

### Bug报告

提交bug报告时，请提供：

- **标题**：清晰描述问题
- **环境信息**：操作系统、编译器版本、SQLCC版本
- **重现步骤**：详细的复现步骤
- **期望行为**：应该发生什么
- **实际行为**：实际发生了什么
- **错误信息**：相关的错误日志或截图

### 功能建议

提出新功能建议时，请说明：

- **问题描述**：当前存在什么问题
- **建议方案**：如何解决这个问题
- **使用场景**：这个功能的应用场景
- **优先级**：这个功能的紧急程度

## 社区规范

### 行为准则

我们致力于维护一个开放、包容的社区环境：

- **尊重他人**：对所有贡献者保持尊重和礼貌
- **建设性反馈**：提供建设性的代码审查意见
- **包容性**：欢迎不同背景和经验水平的贡献者
- **专业性**：保持专业的工作态度和沟通方式

### 沟通渠道

- **GitHub Issues**：报告bug和提出建议
- **GitHub Discussions**：技术讨论和问题解答
- **邮件列表**：重要公告和技术讨论

## 贡献者奖励

### 贡献认可

我们重视每一位贡献者的努力：

- **Contributors列表**：在README中列出主要贡献者
- **Release Notes**：在版本发布说明中感谢贡献者
- **社区活动**：定期举办社区活动和线上分享

### 贡献等级

根据贡献程度，我们设置了不同的贡献者等级：

- **Contributor**：首次代码贡献
- **Active Contributor**：多次贡献，参与代码审查
- **Maintainer**：核心维护者，负责重要模块
- **Core Team**：项目核心团队成员

## 常见问题

### Q: 如何选择合适的提交类型？
A: 根据修改的主要内容选择：新功能用`feat`，修复bug用`fix`，文档更新用`docs`。

### Q: 代码没有通过CI检查怎么办？
A: 检查错误信息，通常是编码规范、测试失败或构建问题。根据提示修复后重新提交。

### Q: 如何处理大型功能开发？
A: 对于复杂功能，建议先创建Issue讨论设计方案，然后分阶段实现，每个阶段创建一个PR。

### Q: 如何处理紧急的bug修复？
A: 紧急bug可以直接在主分支上修复，但仍需要通过代码审查流程。

## 联系我们

- **项目主页**：https://gitee.com/yinglichina/sqlcc
- **文档中心**：https://sqlcc-docs.readthedocs.io/
- **邮件列表**：sqlcc-dev@groups.io
- **社区论坛**：https://forum.sqlcc.org/

感谢您的贡献，让我们一起构建更好的SQLCC！

---

*最后更新: 2025-12-23*
