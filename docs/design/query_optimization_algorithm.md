# SQLCC查询优化算法详解 - 代价模型、连接顺序选择与索引优化策略

## 引言

查询优化器是数据库系统的"大脑"，负责将逻辑查询计划转换为高效的物理执行计划。SQLCC查询优化器实现了完整的优化框架，包括基于规则的优化、基于代价的优化、自适应优化等。本文档将深入分析查询优化的核心算法和实现机制。

## 1. 查询优化理论基础

### 1.1 优化层次架构

**Why层 - 查询优化的重要性：**
数据库查询性能差异可达数千倍，优化器的选择直接决定了系统性能：
- **执行时间**：从秒级到毫秒级的性能提升
- **资源利用**：CPU、内存、I/O的优化使用
- **并发能力**：减少锁竞争和等待时间
- **用户体验**：快速响应提升用户满意度

**优化层次分类：**
1. **语法优化**：消除冗余谓词和子句
2. **逻辑优化**：等价变换，选择最佳逻辑算子
3. **物理优化**：选择具体算法和访问路径
4. **运行时优化**：根据统计信息动态调整

### 1.2 代价模型设计

**代价估算框架：**
```cpp
class CostModel {
public:
    // 估算算子执行代价
    double EstimateOperatorCost(const Operator& op, const Statistics& stats) {
        switch (op.type) {
            case OperatorType::SCAN:
                return EstimateScanCost(op, stats);
            case OperatorType::JOIN:
                return EstimateJoinCost(op, stats);
            case OperatorType::SORT:
                return EstimateSortCost(op, stats);
            case OperatorType::AGGREGATE:
                return EstimateAggregateCost(op, stats);
            default:
                return DEFAULT_COST;
        }
    }

private:
    // 扫描代价估算
    double EstimateScanCost(const Operator& scan_op, const Statistics& stats) {
        const ScanOperator& scan = static_cast<const ScanOperator&>(scan_op);

        // 表扫描 vs 索引扫描
        if (scan.uses_index) {
            // 索引扫描：I/O代价 + CPU代价
            double selectivity = CalculateSelectivity(scan.predicates, stats);
            double rows_to_read = stats.table_size * selectivity;
            return rows_to_read * INDEX_IO_COST + rows_to_read * CPU_COST;
        } else {
            // 表扫描：顺序I/O
            return stats.table_size * SEQUENTIAL_IO_COST;
        }
    }

    // 连接代价估算
    double EstimateJoinCost(const Operator& join_op, const Statistics& stats) {
        const JoinOperator& join = static_cast<const JoinOperator&>(join_op);

        size_t left_rows = join.left->EstimateCardinality();
        size_t right_rows = join.right->EstimateCardinality();

        switch (join.algorithm) {
            case JoinAlgorithm::NESTED_LOOP:
                return left_rows * right_rows * TUPLE_COMPARE_COST;
            case JoinAlgorithm::HASH_JOIN:
                return left_rows + right_rows + (left_rows * HASH_COST);
            case JoinAlgorithm::MERGE_JOIN:
                return left_rows + right_rows + (left_rows * SORT_COST);
            default:
                return left_rows * right_rows;
        }
    }
};
```

**统计信息收集：**
```cpp
class StatisticsCollector {
public:
    TableStatistics CollectTableStatistics(const std::string& table_name) {
        TableStatistics stats;

        // 1. 基本表信息
        stats.table_size = GetTableRowCount(table_name);
        stats.page_count = GetTablePageCount(table_name);

        // 2. 列统计信息
        for (const auto& column : GetTableColumns(table_name)) {
            ColumnStatistics col_stats = CollectColumnStatistics(table_name, column);
            stats.column_stats[column.name] = col_stats;
        }

        // 3. 索引统计信息
        for (const auto& index : GetTableIndexes(table_name)) {
            IndexStatistics idx_stats = CollectIndexStatistics(index);
            stats.index_stats[index.name] = idx_stats;
        }

        return stats;
    }

private:
    ColumnStatistics CollectColumnStatistics(const std::string& table, const Column& col) {
        ColumnStatistics stats;

        // 基数（不同值的数量）
        stats.cardinality = CalculateColumnCardinality(table, col);

        // 直方图
        stats.histogram = BuildColumnHistogram(table, col, HISTOGRAM_BUCKETS);

        // NULL值比例
        stats.null_fraction = CalculateNullFraction(table, col);

        // 最常见值
        stats.most_common_values = FindMostCommonValues(table, col, TOP_K_VALUES);

        return stats;
    }
};
```

## 2. 基于规则的优化

### 2.1 关系代数变换规则

**选择下推规则：**
```cpp
class SelectionPushdownRule : public OptimizationRule {
public:
    bool Apply(OperatorTree& tree) override {
        return PushSelectionsDown(tree.root);
    }

private:
    bool PushSelectionsDown(OperatorNode* node) {
        bool changed = false;

        if (node->type == OperatorType::SELECT) {
            // 尝试将选择条件推向下层算子
            changed |= TryPushSelectionDown(node);
        }

        // 递归处理子节点
        for (auto child : node->children) {
            changed |= PushSelectionsDown(child);
        }

        return changed;
    }

    bool TryPushSelectionDown(OperatorNode* select_node) {
        // 检查选择条件是否可以推到子节点
        for (const auto& predicate : select_node->predicates) {
            if (CanPushPredicate(predicate, select_node->children[0])) {
                // 执行下推
                PushPredicateToChild(predicate, select_node, select_node->children[0]);
                return true;
            }
        }
        return false;
    }

    bool CanPushPredicate(const Predicate& pred, OperatorNode* child) {
        // 检查谓词是否只涉及子节点的属性
        std::set<std::string> pred_attrs = GetPredicateAttributes(pred);
        std::set<std::string> child_attrs = GetOperatorAttributes(child);

        return std::includes(child_attrs.begin(), child_attrs.end(),
                           pred_attrs.begin(), pred_attrs.end());
    }
};
```

**投影下推规则：**
```cpp
class ProjectionPushdownRule : public OptimizationRule {
public:
    bool Apply(OperatorTree& tree) override {
        return PushProjectionsDown(tree.root);
    }

private:
    bool PushProjectionsDown(OperatorNode* node) {
        bool changed = false;

        if (node->type == OperatorType::PROJECT) {
            // 尝试将投影属性推向下层算子
            changed |= TryPushProjectionDown(node);
        }

        // 递归处理子节点
        for (auto child : node->children) {
            changed |= PushProjectionsDown(child);
        }

        return changed;
    }

    bool TryPushProjectionDown(OperatorNode* project_node) {
        // 检查投影是否可以推到子节点
        std::set<std::string> project_attrs = project_node->projected_attributes;
        std::set<std::string> child_attrs = GetOperatorAttributes(project_node->children[0]);

        if (std::includes(child_attrs.begin(), child_attrs.end(),
                         project_attrs.begin(), project_attrs.end())) {
            // 执行下推
            PushProjectionToChild(project_attrs, project_node, project_node->children[0]);
            return true;
        }

        return false;
    }
};
```

### 2.2 连接顺序优化

**动态规划算法：**
```cpp
class JoinOrderOptimizer {
public:
    QueryPlan OptimizeJoinOrder(const std::vector<Relation>& relations,
                               const std::vector<JoinCondition>& conditions) {

        size_t n = relations.size();
        std::vector<std::vector<QueryPlan>> best_plans(n + 1);

        // 初始化单个关系的最佳计划
        for (size_t i = 0; i < n; ++i) {
            best_plans[1][i] = CreateBasePlan(relations[i]);
        }

        // 动态规划求解
        for (size_t subset_size = 2; subset_size <= n; ++subset_size) {
            for (auto subset : GenerateSubsets(relations, subset_size)) {
                for (auto split : GenerateSplits(subset)) {
                    // 计算左子集和右子集的连接代价
                    QueryPlan left_plan = best_plans[split.left.size()][split.left];
                    QueryPlan right_plan = best_plans[split.right.size()][split.right];

                    double join_cost = EstimateJoinCost(left_plan, right_plan, split.condition);

                    // 更新最优计划
                    size_t subset_key = GetSubsetKey(subset);
                    if (join_cost < best_costs[subset_key]) {
                        best_plans[subset_size][subset_key] =
                            CreateJoinPlan(left_plan, right_plan, split.condition);
                        best_costs[subset_key] = join_cost;
                    }
                }
            }
        }

        return best_plans[n][GetFullSetKey(relations)];
    }

private:
    // 状态压缩：使用位掩码表示关系子集
    size_t GetSubsetKey(const std::vector<Relation>& subset) {
        size_t key = 0;
        for (const auto& rel : subset) {
            key |= (1ULL << rel.id);
        }
        return key;
    }

    std::vector<JoinSplit> GenerateSplits(const std::vector<Relation>& subset) {
        std::vector<JoinSplit> splits;

        // 枚举所有可能的分割
        size_t n = subset.size();
        for (size_t mask = 1; mask < (1ULL << (n - 1)); ++mask) {
            std::vector<Relation> left, right;

            for (size_t i = 0; i < n; ++i) {
                if (mask & (1ULL << i)) {
                    left.push_back(subset[i]);
                } else {
                    right.push_back(subset[i]);
                }
            }

            // 查找连接条件
            auto condition = FindJoinCondition(left, right);
            if (condition) {
                splits.push_back({left, right, *condition});
            }
        }

        return splits;
    }
};
```

## 3. 索引选择优化

### 3.1 索引选择算法

**多索引选择策略：**
```cpp
class IndexSelector {
public:
    std::vector<Index*> SelectOptimalIndexes(const Query& query,
                                           const Workload& workload,
                                           const Statistics& stats) {

        // 1. 分析查询谓词
        auto predicates = ExtractPredicates(query);

        // 2. 评估候选索引
        std::vector<IndexCandidate> candidates;
        for (const auto& index : available_indexes) {
            double score = EvaluateIndexForQuery(index, predicates, stats);
            candidates.push_back({index, score});
        }

        // 3. 选择最优索引组合
        return SelectIndexCombination(candidates, query, stats);
    }

private:
    double EvaluateIndexForQuery(const Index* index,
                                const std::vector<Predicate>& predicates,
                                const Statistics& stats) {

        double score = 0.0;

        // 1. 匹配度评分
        score += CalculatePredicateMatchScore(index, predicates) * 0.4;

        // 2. 选择性评分
        score += CalculateIndexSelectivity(index, predicates, stats) * 0.3;

        // 3. 维护代价评分
        score += CalculateMaintenanceCost(index, workload) * 0.2;

        // 4. 存储代价评分
        score += CalculateStorageCost(index) * 0.1;

        return score;
    }

    double CalculatePredicateMatchScore(const Index* index,
                                       const std::vector<Predicate>& predicates) {
        double match_score = 0.0;
        auto index_columns = index->GetColumns();

        for (const auto& pred : predicates) {
            // 检查谓词是否可以使用索引
            if (CanUseIndexForPredicate(index, pred)) {
                match_score += 1.0;

                // 前缀匹配额外加分
                if (IsPrefixMatch(index_columns, pred)) {
                    match_score += 0.5;
                }
            }
        }

        return match_score / predicates.size();
    }

    double CalculateIndexSelectivity(const Index* index,
                                   const std::vector<Predicate>& predicates,
                                   const Statistics& stats) {
        double selectivity = 1.0;

        for (const auto& pred : predicates) {
            if (CanUseIndexForPredicate(index, pred)) {
                selectivity *= EstimatePredicateSelectivity(pred, stats);
            }
        }

        // 选择性越低（过滤效果越好），评分越高
        return 1.0 - selectivity;
    }
};
```

### 3.2 复合索引设计

**索引键顺序优化：**
```cpp
class CompositeIndexDesigner {
public:
    std::vector<ColumnOrder> DesignCompositeIndex(const Workload& workload,
                                                const Table& table) {

        // 1. 收集查询模式
        auto query_patterns = AnalyzeQueryPatterns(workload);

        // 2. 计算列的相关性
        auto column_affinity = CalculateColumnAffinity(query_patterns);

        // 3. 生成候选索引
        std::vector<CompositeIndex> candidates;
        GenerateIndexCandidates(table.columns, column_affinity, candidates);

        // 4. 评估和排序候选索引
        return RankIndexCandidates(candidates, workload);
    }

private:
    struct ColumnAffinity {
        std::string column1;
        std::string column2;
        double affinity_score;  // 相关性得分
    };

    std::vector<ColumnAffinity> CalculateColumnAffinity(
        const std::vector<QueryPattern>& patterns) {

        std::vector<ColumnAffinity> affinities;

        // 分析列在查询中的共现频率
        for (const auto& pattern : patterns) {
            auto columns = ExtractColumnsFromPattern(pattern);

            // 计算列对的相关性
            for (size_t i = 0; i < columns.size(); ++i) {
                for (size_t j = i + 1; j < columns.size(); ++j) {
                    double affinity = CalculatePairAffinity(columns[i], columns[j], patterns);
                    affinities.push_back({columns[i], columns[j], affinity});
                }
            }
        }

        return affinities;
    }

    double CalculatePairAffinity(const std::string& col1, const std::string& col2,
                               const std::vector<QueryPattern>& patterns) {
        size_t both_present = 0;
        size_t either_present = 0;

        for (const auto& pattern : patterns) {
            bool has_col1 = pattern.columns.count(col1);
            bool has_col2 = pattern.columns.count(col2);

            if (has_col1 && has_col2) both_present++;
            if (has_col1 || has_col2) either_present++;
        }

        // 使用Jaccard相似系数
        return static_cast<double>(both_present) / either_present;
    }
};
```

## 4. 自适应查询优化

### 4.1 运行时统计收集

**查询执行统计：**
```cpp
class QueryExecutionMonitor {
public:
    void RecordQueryExecution(const Query& query, const ExecutionStats& stats) {
        // 1. 更新查询统计信息
        UpdateQueryStatistics(query, stats);

        // 2. 更新操作符统计信息
        UpdateOperatorStatistics(query.plan, stats);

        // 3. 检查是否需要重新优化
        if (ShouldReoptimize(query, stats)) {
            TriggerReoptimization(query);
        }
    }

private:
    void UpdateQueryStatistics(const Query& query, const ExecutionStats& stats) {
        auto& query_stats = query_statistics[query.signature];

        query_stats.execution_count++;
        query_stats.total_execution_time += stats.execution_time;
        query_stats.total_io_operations += stats.io_operations;

        // 更新移动平均
        query_stats.avg_execution_time =
            (query_stats.avg_execution_time * (query_stats.execution_count - 1) +
             stats.execution_time) / query_stats.execution_count;
    }

    bool ShouldReoptimize(const Query& query, const ExecutionStats& stats) {
        const auto& query_stats = query_statistics[query.signature];

        // 检查执行时间是否显著偏离历史平均值
        double deviation = std::abs(stats.execution_time - query_stats.avg_execution_time);
        double threshold = query_stats.avg_execution_time * REOPTIMIZATION_THRESHOLD;

        return deviation > threshold;
    }
};
```

### 4.2 动态代价校准

**自适应代价模型：**
```cpp
class AdaptiveCostModel : public CostModel {
public:
    void CalibrateCosts(const Query& query, const ExecutionStats& actual_stats) {
        // 1. 计算预测误差
        double predicted_cost = EstimateCost(query.plan);
        double actual_cost = CalculateActualCost(actual_stats);
        double error_ratio = actual_cost / predicted_cost;

        // 2. 更新代价参数
        if (error_ratio > COST_CALIBRATION_THRESHOLD) {
            UpdateCostParameters(query.plan, error_ratio);
        }

        // 3. 记录校准历史
        RecordCalibrationEvent(query, predicted_cost, actual_cost);
    }

private:
    void UpdateCostParameters(const QueryPlan& plan, double error_ratio) {
        // 根据误差调整代价模型参数
        for (const auto& op : plan.operators) {
            switch (op.type) {
                case OperatorType::SCAN:
                    AdjustScanCostParameters(op, error_ratio);
                    break;
                case OperatorType::JOIN:
                    AdjustJoinCostParameters(op, error_ratio);
                    break;
                case OperatorType::SORT:
                    AdjustSortCostParameters(op, error_ratio);
                    break;
            }
        }
    }

    void AdjustScanCostParameters(const Operator& op, double error_ratio) {
        // 调整I/O代价因子
        if (op.uses_index) {
            index_io_cost_factor *= (1.0 + LEARNING_RATE * (error_ratio - 1.0));
        } else {
            table_scan_cost_factor *= (1.0 + LEARNING_RATE * (error_ratio - 1.0));
        }
    }
};
```

### 4.3 多目标优化

**Pareto最优解选择：**
```cpp
class MultiObjectiveOptimizer {
public:
    QueryPlan SelectParetoOptimalPlan(const std::vector<QueryPlan>& candidates,
                                    const OptimizationObjectives& objectives) {

        // 1. 计算每个候选计划的目标值
        std::vector<PlanObjectives> plan_scores;
        for (const auto& plan : candidates) {
            PlanObjectives scores = EvaluatePlanObjectives(plan, objectives);
            plan_scores.push_back(scores);
        }

        // 2. 识别Pareto最优解
        std::vector<size_t> pareto_front = IdentifyParetoFront(plan_scores);

        // 3. 从Pareto前沿中选择最优解
        return SelectBestFromParetoFront(candidates, pareto_front, objectives);
    }

private:
    struct PlanObjectives {
        double execution_cost;    // 执行代价
        double memory_usage;      // 内存使用
        double cpu_usage;         // CPU使用率
        double robustness;        // 鲁棒性评分
    };

    std::vector<size_t> IdentifyParetoFront(const std::vector<PlanObjectives>& scores) {
        std::vector<size_t> pareto_front;

        for (size_t i = 0; i < scores.size(); ++i) {
            bool is_dominated = false;

            // 检查是否有其他解支配当前解
            for (size_t j = 0; j < scores.size(); ++j) {
                if (i != j && Dominates(scores[j], scores[i])) {
                    is_dominated = true;
                    break;
                }
            }

            if (!is_dominated) {
                pareto_front.push_back(i);
            }
        }

        return pareto_front;
    }

    bool Dominates(const PlanObjectives& a, const PlanObjectives& b) {
        // 检查a是否在所有目标上都不比b差，且至少在一个目标上比b好
        bool at_least_one_better = false;

        if (a.execution_cost <= b.execution_cost) {
            if (a.execution_cost < b.execution_cost) at_least_one_better = true;
        } else {
            return false;
        }

        if (a.memory_usage <= b.memory_usage) {
            if (a.memory_usage < b.memory_usage) at_least_one_better = true;
        } else {
            return false;
        }

        // 类似检查其他目标...

        return at_least_one_better;
    }
};
```

## 5. 性能测试与验证

### 5.1 优化器正确性测试

**等价变换验证：**
```cpp
class OptimizerCorrectnessTest {
public:
    void TestTransformationEquivalence() {
        // 生成测试查询
        auto test_queries = GenerateTestQueries();

        for (const auto& query : test_queries) {
            // 1. 生成原始执行计划
            auto original_plan = GeneratePlanWithoutOptimization(query);

            // 2. 应用优化规则
            auto optimized_plan = optimizer.Optimize(query);

            // 3. 验证结果等价性
            assert(PlansAreEquivalent(original_plan, optimized_plan));

            // 4. 验证优化后性能更好
            assert(MeasurePlanCost(optimized_plan) < MeasurePlanCost(original_plan));
        }
    }

private:
    bool PlansAreEquivalent(const QueryPlan& plan1, const QueryPlan& plan2) {
        // 1. 执行两个计划并比较结果
        auto result1 = ExecutePlan(plan1, test_data);
        auto result2 = ExecutePlan(plan2, test_data);

        // 2. 结果应该完全相同（考虑排序）
        return ResultsAreEqual(result1, result2);
    }

    double MeasurePlanCost(const QueryPlan& plan) {
        // 使用代价模型估算执行代价
        return cost_model.EstimateCost(plan);
    }
};
```

### 5.2 性能回归测试

**端到端性能测试：**
```cpp
class PerformanceRegressionTest {
public:
    void RunPerformanceRegressionSuite() {
        auto test_queries = LoadBenchmarkQueries();

        for (const auto& query : test_queries) {
            // 1. 记录基准性能
            double baseline_time = MeasureExecutionTime(query, baseline_optimizer);

            // 2. 测试新优化器性能
            double new_time = MeasureExecutionTime(query, new_optimizer);

            // 3. 检查性能回归
            double slowdown_ratio = new_time / baseline_time;
            if (slowdown_ratio > PERFORMANCE_REGRESSION_THRESHOLD) {
                ReportPerformanceRegression(query, slowdown_ratio);
            }

            // 4. 记录性能提升
            if (baseline_time > new_time) {
                double speedup = baseline_time / new_time;
                ReportPerformanceImprovement(query, speedup);
            }
        }
    }

private:
    double MeasureExecutionTime(const Query& query, QueryOptimizer& optimizer) {
        // 预热
        for (int i = 0; i < WARMUP_RUNS; ++i) {
            optimizer.Optimize(query);
        }

        // 正式测试
        auto start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < MEASUREMENT_RUNS; ++i) {
            auto plan = optimizer.Optimize(query);
            ExecutePlan(plan);  // 实际执行以获得准确时间
        }

        auto end = std::chrono::high_resolution_clock::now();

        return std::chrono::duration<double, std::milli>(end - start).count() / MEASUREMENT_RUNS;
    }
};
```

## 6. 总结与展望

SQLCC查询优化器通过精心设计的代价模型、优化算法和自适应机制，在查询性能优化方面达到了工业级标准。

**核心成就：**
- **优化效率**：基于代价的智能查询优化
- **自适应能力**：运行时统计信息驱动的优化
- **多目标平衡**：性能、内存、鲁棒性的综合优化
- **可扩展架构**：插件化的优化规则和策略

**未来优化方向：**
- **机器学习优化**：AI驱动的查询计划选择
- **分布式查询优化**：跨节点查询的全局优化
- **实时优化**：流式数据处理的实时优化
- **硬件加速**：利用GPU等硬件加速优化过程

---

*文档创建时间: 2025-12-24*
*作者: SQLCC技术委员会*
*版本: v1.2.6*
