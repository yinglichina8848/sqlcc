#include "execution/window_function_executor.h"
#include <algorithm>
#include <iostream>

namespace sqlcc {

WindowFunctionExecutor::WindowFunctionExecutor(std::shared_ptr<DatabaseManager> db_manager)
    : db_manager_(db_manager) {}

WindowFunctionResult WindowFunctionExecutor::executeWindowFunction(
    const sql_parser::WindowFunction& window_function,
    const std::string& table_name,
    const std::vector<std::vector<std::string>>& data) {
    
    switch (window_function.getFunctionType()) {
        case sql_parser::WindowFunction::ROW_NUMBER:
            return executeRowNumber(window_function, data);
            
        case sql_parser::WindowFunction::RANK:
            return executeRank(window_function, data);
            
        case sql_parser::WindowFunction::DENSE_RANK:
            return executeDenseRank(window_function, data);
            
        default:
            WindowFunctionResult result;
            result.success = false;
            result.error_message = "Unsupported window function type";
            return result;
    }
}

WindowFunctionResult WindowFunctionExecutor::executeRowNumber(
    const sql_parser::WindowFunction& window_function,
    const std::vector<std::vector<std::string>>& data) {
    
    WindowFunctionResult result;
    result.success = true;
    
    // 获取排序索引
    auto sorted_indices = sortData(
        window_function.getPartitionByColumns(),
        window_function.getOrderByColumns(),
        window_function.getOrderDirection(),
        data);
    
    // 为每个分区计算行号
    result.values.resize(data.size());
    std::vector<std::string> partition_keys(data.size());
    
    // 构建分区键
    for (size_t i = 0; i < data.size(); ++i) {
        std::string key;
        for (const auto& col : window_function.getPartitionByColumns()) {
            key += col + "|";
        }
        partition_keys[i] = key;
    }
    
    // 计算行号
    std::string current_partition;
    int row_number = 0;
    
    for (size_t i = 0; i < sorted_indices.size(); ++i) {
        size_t idx = sorted_indices[i];
        
        // 检查是否是新分区
        if (partition_keys[idx] != current_partition) {
            current_partition = partition_keys[idx];
            row_number = 1;
        } else {
            row_number++;
        }
        
        result.values[idx] = std::to_string(row_number);
    }
    
    return result;
}

WindowFunctionResult WindowFunctionExecutor::executeRank(
    const sql_parser::WindowFunction& window_function,
    const std::vector<std::vector<std::string>>& data) {
    
    WindowFunctionResult result;
    result.success = true;
    
    // 获取排序索引
    auto sorted_indices = sortData(
        window_function.getPartitionByColumns(),
        window_function.getOrderByColumns(),
        window_function.getOrderDirection(),
        data);
    
    // 为每个分区计算排名
    result.values.resize(data.size());
    std::vector<std::string> partition_keys(data.size());
    std::vector<std::string> order_keys(data.size());
    
    // 构建分区键和排序键
    for (size_t i = 0; i < data.size(); ++i) {
        std::string partition_key;
        for (const auto& col : window_function.getPartitionByColumns()) {
            partition_key += col + "|";
        }
        partition_keys[i] = partition_key;
        
        std::string order_key;
        for (const auto& col : window_function.getOrderByColumns()) {
            order_key += col + "|";
        }
        order_keys[i] = order_key;
    }
    
    // 计算排名
    std::string current_partition;
    std::string current_order_key;
    int rank = 0;
    int row_number = 0;
    
    for (size_t i = 0; i < sorted_indices.size(); ++i) {
        size_t idx = sorted_indices[i];
        row_number++;
        
        // 检查是否是新分区
        if (partition_keys[idx] != current_partition) {
            current_partition = partition_keys[idx];
            current_order_key = "";
            rank = row_number;
        } 
        // 检查是否是新排序值
        else if (order_keys[idx] != current_order_key) {
            rank = row_number;
        }
        
        current_order_key = order_keys[idx];
        result.values[idx] = std::to_string(rank);
    }
    
    return result;
}

WindowFunctionResult WindowFunctionExecutor::executeDenseRank(
    const sql_parser::WindowFunction& window_function,
    const std::vector<std::vector<std::string>>& data) {
    
    WindowFunctionResult result;
    result.success = true;
    
    // 获取排序索引
    auto sorted_indices = sortData(
        window_function.getPartitionByColumns(),
        window_function.getOrderByColumns(),
        window_function.getOrderDirection(),
        data);
    
    // 为每个分区计算密集排名
    result.values.resize(data.size());
    std::vector<std::string> partition_keys(data.size());
    std::vector<std::string> order_keys(data.size());
    
    // 构建分区键和排序键
    for (size_t i = 0; i < data.size(); ++i) {
        std::string partition_key;
        for (const auto& col : window_function.getPartitionByColumns()) {
            partition_key += col + "|";
        }
        partition_keys[i] = partition_key;
        
        std::string order_key;
        for (const auto& col : window_function.getOrderByColumns()) {
            order_key += col + "|";
        }
        order_keys[i] = order_key;
    }
    
    // 计算密集排名
    std::string current_partition;
    std::string current_order_key;
    int dense_rank = 0;
    
    for (size_t i = 0; i < sorted_indices.size(); ++i) {
        size_t idx = sorted_indices[i];
        
        // 检查是否是新分区
        if (partition_keys[idx] != current_partition) {
            current_partition = partition_keys[idx];
            current_order_key = "";
            dense_rank = 1;
        } 
        // 检查是否是新排序值
        else if (order_keys[idx] != current_order_key) {
            dense_rank++;
        }
        
        current_order_key = order_keys[idx];
        result.values[idx] = std::to_string(dense_rank);
    }
    
    return result;
}

std::vector<size_t> WindowFunctionExecutor::sortData(
    const std::vector<std::string>& partition_columns,
    const std::vector<std::string>& order_columns,
    const std::string& order_direction,
    const std::vector<std::vector<std::string>>& data) {
    
    std::vector<size_t> indices(data.size());
    for (size_t i = 0; i < data.size(); ++i) {
        indices[i] = i;
    }
    
    // 简化的排序实现，实际应用中需要根据具体的列索引进行排序
    std::sort(indices.begin(), indices.end(), [&](size_t a, size_t b) {
        // 首先按分区列排序
        for (const auto& col : partition_columns) {
            // 这里简化处理，实际需要根据列名找到对应的数据列
            if (data[a][0] != data[b][0]) {
                return data[a][0] < data[b][0];
            }
        }
        
        // 然后按排序列排序
        for (const auto& col : order_columns) {
            // 这里简化处理，实际需要根据列名找到对应的数据列
            if (data[a][0] != data[b][0]) {
                if (order_direction == "DESC") {
                    return data[a][0] > data[b][0];
                } else {
                    return data[a][0] < data[b][0];
                }
            }
        }
        
        return a < b; // 保持稳定排序
    });
    
    return indices;
}

} // namespace sqlcc
