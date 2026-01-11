#ifndef SQLCC_STORAGE_BUFFER_POOL_H
#define SQLCC_STORAGE_BUFFER_POOL_H

/**
 * WHY: 为什么需要缓冲池别名头文件？
 *
 * 数据库系统在重构过程中可能需要保持向后兼容性：
 * - 旧版本代码可能引用旧的头文件名
 * - 渐进式重构需要保持API兼容性
 * - 避免大规模代码修改导致的编译错误
 *
 * 🏗️ 设计模式：适配器模式(Adapter Pattern)
 * - 为新接口提供旧接口的别名
 * - 保持向后兼容性
 * - 简化重构过程
 *
 * WHAT: 缓冲池别名头文件
 * - 为buffer_pool_sharded.h提供兼容性别名
 * - 保持API向后兼容性
 *
 * HOW: 别名实现机制
 * - 直接包含实际的头文件
 * - 提供完全相同的接口
 * - 零成本抽象
 */

// 为兼容性提供 buffer_pool.h 别名
#include "buffer_pool_sharded.h"

#endif // SQLCC_STORAGE_BUFFER_POOL_H
