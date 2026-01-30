/**
 * WHY: 为什么需要专门的LOAD DATA执行器？
 *
 * 数据库系统需要高效地处理大量数据的批量导入，传统方案存在诸多问题：
 * - 数据导入效率低下：单行插入导致性能瓶颈和事务开销巨大
 * - 数据格式解析复杂：缺乏统一的CSV、TSV等格式解析能力
 * - 错误处理不完善：数据导入失败后缺乏有效的错误恢复机制
 * - 内存使用不合理：大文件导入时容易造成内存溢出或性能下降
 * - 并发控制缺失：多个导入任务同时执行时的资源竞争问题
 * - 数据一致性保证：导入过程中数据一致性和约束检查的复杂性
 * - 权限控制不严：数据导入缺乏细粒度的权限验证机制
 *
 * LOAD DATA执行器的核心价值：
 * 1. 高性能批量导入：大幅提升数据导入速度，减少事务开销
 * 2. 多格式数据支持：支持CSV、TSV、自定义分隔符等多种数据格式
 * 3. 智能错误处理：完善的错误检测、跳过、恢复机制
 * 4. 内存优化管理：流式处理大文件，避免内存溢出风险
 * 5. 并发安全控制：多任务并发导入时的资源协调和隔离
 * 6. 数据质量保证：导入过程中的数据验证、转换、约束检查
 * 7. 权限安全隔离：基于用户的导入权限控制和审计日志
 *
 * 🏗️ 设计模式：模板方法模式(Template Method Pattern) + 策略模式(Strategy Pattern) + 状态机模式(State Machine Pattern)
 *
 * LOAD DATA执行器作为模板方法模式的应用：
 * - 算法骨架定义：定义数据导入的通用流程和步骤
 * - 具体步骤定制：允许子类定制特定的解析、验证、插入逻辑
 * - 流程控制统一：保证导入流程的一致性和可预测性
 * - 扩展性保证：新数据格式的支持通过模板方法扩展实现
 * - 代码复用优化：避免重复实现相似的导入逻辑
 *
 * SOLID原则体现：
 * - 单一职责：LOAD DATA执行器专门负责数据批量导入功能
 * - 开闭原则：新数据格式通过扩展现有类实现
 * - 里氏替换：不同格式的执行器可以互相替换
 * - 接口隔离：导入接口精确定义导入契约
 * - 依赖倒置：执行器依赖抽象的数据源和存储接口
 *
 * WHAT: LOAD DATA执行器系统 - 数据库批量数据导入框架
 *
 * 核心功能：
 * - 文件数据读取：支持本地文件和服务器文件的读取
 * - 数据格式解析：支持多种分隔符、引号包围、转义字符处理
 * - 数据验证转换：数据类型验证、格式转换、约束检查
 * - 批量数据插入：高效的批量插入和事务管理
 * - 错误处理恢复：错误检测、跳过、警告、失败恢复机制
 * - 统计信息收集：导入过程的详细统计和性能监控
 * - 权限访问控制：导入权限验证和安全审计
 *
 * 系统组件：
 * - LoadDataExecutor：核心执行器，协调整个导入过程
 * - FileReader：文件读取器，支持多种文件访问方式
 * - DataParser：数据解析器，处理各种数据格式
 * - DataValidator：数据验证器，检查数据质量和约束
 * - BatchInserter：批量插入器，优化数据插入性能
 * - ErrorHandler：错误处理器，管理导入过程中的异常情况
 * - StatisticsCollector：统计收集器，记录导入过程的详细信息
 *
 * 支持的数据格式：
 * - CSV格式：逗号分隔值，标准CSV文件格式
 * - TSV格式：制表符分隔值，TSV文件格式
 * - 自定义分隔符：用户指定任意分隔符
 * - 固定宽度：固定列宽的数据文件
 * - JSON Lines：每行一个JSON对象的格式
 * - XML格式：XML结构化数据导入
 * - 二进制格式：自定义二进制数据格式
 *
 * 数据导入流程：
 * - 文件访问验证：检查文件存在性和访问权限
 * - 格式参数解析：解析分隔符、引号、转义等格式参数
 * - 表结构获取：获取目标表的元数据信息
 * - 数据流读取：按行或按块读取文件数据
 * - 数据解析处理：解析字段、处理引号和转义
 * - 数据验证转换：类型检查、格式转换、约束验证
 * - 表达式计算：处理SET子句中的表达式计算
 * - 批量插入执行：高效的批量数据插入
 * - 错误统计报告：收集和报告导入过程中的问题
 *
 * 性能优化策略：
 * - 流式处理：大文件的分块读取和处理
 * - 批量提交：减少事务提交次数，提高性能
 * - 并行处理：多线程并发处理，提高CPU利用率
 * - 内存复用：对象池和缓冲区复用，减少GC压力
 * - 索引优化：导入过程中的索引维护策略优化
 * - 缓存机制：元数据和解析结果的缓存优化
 *
 * 错误处理机制：
 * - 数据格式错误：字段数量不匹配、类型转换失败等
 * - 约束违反错误：主键冲突、检查约束失败等
 * - 文件访问错误：文件不存在、权限不足等
 * - 内存不足错误：大文件处理时的内存溢出
 * - 编码转换错误：字符集转换问题
 * - 网络超时错误：远程文件访问超时
 *
 * 安全控制策略：
 * - 文件路径验证：防止路径遍历和目录穿越攻击
 * - 文件大小限制：防止恶意的大文件上传
 * - 数据内容过滤：防止SQL注入和恶意数据
 * - 用户权限检查：验证用户对表的导入权限
 * - 审计日志记录：记录所有的导入操作和异常情况
 * - 资源使用限制：限制导入过程的CPU、内存、I/O使用
 *
 * 接口设计：
 * - 执行接口：LOAD DATA语句的主要执行接口
 * - 配置接口：导入参数和策略的配置接口
 * - 监控接口：导入过程性能和状态的监控接口
 * - 回调接口：导入过程各个阶段的回调接口
 *
 * HOW: LOAD DATA执行器系统的实现机制
 *
 * 模板方法模式实现：
 * 1. 算法骨架：定义数据导入的固定流程步骤
 * 2. 钩子方法：允许子类在关键步骤插入自定义逻辑
 * 3. 抽象方法：定义子类必须实现的解析和插入方法
 * 4. 具体实现：各种数据格式的具体导入实现
 * 5. 流程控制：统一的流程控制和错误处理机制
 *
 * 数据解析实现：
 * 1. 分隔符识别：智能识别和处理各种分隔符
 * 2. 引号处理：正确处理引号包围和转义字符
 * 3. 换行符处理：处理跨行的字段数据
 * 4. 空值处理：识别和处理NULL值的各种表示形式
 * 5. 编码转换：处理不同字符集的转换
 * 6. 数据验证：类型检查和格式验证
 *
 * 批量插入实现：
 * 1. 事务管理：合理控制事务大小和提交频率
 * 2. 批量准备：预编译SQL语句提高性能
 * 3. 参数绑定：高效的参数绑定和执行
 * 4. 错误恢复：失败时的回滚和重试机制
 * 5. 进度监控：导入进度的实时监控和报告
 * 6. 资源清理：及时清理临时资源和连接
 *
 * 内存管理实现：
 * 1. 流式读取：按需读取文件，避免大文件内存占用
 * 2. 缓冲区复用：复用解析缓冲区减少内存分配
 * 3. 对象池管理：连接池和对象池的复用
 * 4. 分页处理：大结果集的分页处理
 * 5. 垃圾回收：及时清理不再使用的对象
 * 6. 内存监控：实时监控内存使用情况
 *
 * 并发控制实现：
 * 1. 文件锁机制：防止多个进程同时导入同一文件
 * 2. 表锁优化：最小化表锁时间，提高并发性
 * 3. 线程池管理：控制并发导入任务的数量
 * 4. 资源隔离：不同导入任务间的资源隔离
 * 5. 死锁预防：避免导入过程中的死锁情况
 * 6. 优先级调度：重要导入任务的优先级处理
 *
 * 错误恢复实现：
 * 1. 错误分类：将错误分为可跳过和致命错误
 * 2. 跳过策略：配置错误的跳过和警告策略
 * 3. 重试机制：网络错误和临时错误的自动重试
 * 4. 回滚处理：失败时的完整事务回滚
 * 5. 恢复点：支持从断点继续导入
 * 6. 日志记录：详细的错误日志和恢复信息
 *
 * 性能监控实现：
 * 1. 速度统计：实时计算导入速度和预计完成时间
 * 2. 内存监控：监控内存使用峰值和趋势
 * 3. CPU监控：监控CPU使用率和热点
 * 4. I/O监控：监控磁盘和网络I/O性能
 * 5. 错误统计：统计各类错误的发生频率
 * 6. 进度报告：定期报告导入进度和状态
 *
 * 扩展性设计：
 * - 插件架构：支持第三方数据格式解析插件
 * - 自定义转换：用户自定义的数据转换函数
 * - 多数据源：支持多种数据源的文件导入
 * - 分布式导入：支持分布式环境下的数据导入
 * - AI优化：基于机器学习的导入优化
 *
 * 调试和诊断：
 * - 执行跟踪：详细记录导入过程的每个步骤
 * - 数据采样：导入数据的采样分析和验证
 * - 性能分析：导入性能的详细分析和优化建议
 * - 错误诊断：错误原因的详细分析和修复建议
 * - 可视化工具：导入过程和结果的可视化展示
 */

#include "sql_parser/ast_nodes.h"
#pragma once

#include <memory>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include "core/execution_result.h"
#include "sql_parser/load_data_ast.h"
#include "../storage_engine/storage_engine.h"
#include "../sql_executor.h"

namespace sqlcc {

class LoadDataExecutor {
public:
    LoadDataExecutor(std::shared_ptr<StorageEngine> storage_engine,
                    std::shared_ptr<SqlExecutor> sql_executor);
    ~LoadDataExecutor();

    // 执行LOAD DATA语句
    ExecutionResult execute(const sql_parser::LoadDataStatement& stmt);

private:
    std::shared_ptr<StorageEngine> storage_engine_;
    std::shared_ptr<SqlExecutor> sql_executor_;

    // 文件处理
    bool openFile(const std::string& filename, bool is_local, std::ifstream& file);
    bool readLine(std::ifstream& file, std::string& line);
    void closeFile(std::ifstream& file);

    // 数据解析
    std::vector<std::string> parseFields(const std::string& line, const sql_parser::LoadDataStatement& stmt);
    std::string unescapeField(const std::string& field, char escape_char);
    std::string removeEnclosure(const std::string& field, const std::string& enclosure, bool optionally);

    // 数据转换和验证
    bool validateAndConvertRow(const std::vector<std::string>& raw_fields,
                              std::vector<std::string>& processed_row,
                              const sql_parser::LoadDataStatement& stmt,
                              std::shared_ptr<TableMetadata> table_meta);
    bool applySetExpressions(std::vector<std::string>& row,
                           const sql_parser::LoadDataStatement& stmt);
    std::string evaluateSetExpression(const std::string& expression,
                                    const std::vector<std::string>& row,
                                    const std::vector<TableColumn>& columns);
    std::string evaluateArithmeticExpression(const std::string& expr);
    bool validateConstraints(const std::vector<std::string>& row,
                           std::shared_ptr<TableMetadata> table_meta);
    bool validateDataType(const std::string& value, const std::string& type);

    // 批量插入
    bool insertRow(const std::vector<std::string>& row,
                  const sql_parser::LoadDataStatement& stmt,
                  std::shared_ptr<TableMetadata> table_meta);

    // 错误处理和统计
    struct LoadStats {
        size_t total_lines = 0;
        size_t skipped_lines = 0;
        size_t inserted_rows = 0;
        size_t failed_rows = 0;
        size_t warnings = 0;
        std::vector<std::string> errors;
    };

    LoadStats stats_;

    // 辅助方法
    std::shared_ptr<TableMetadata> getTableMetadata(const std::string& table_name);
    bool checkTableExists(const std::string& table_name);
    bool checkFilePermissions(const std::string& filename, bool is_local);
    void logWarning(const std::string& message);
    void logError(const std::string& message);
};

} // namespace sqlcc
