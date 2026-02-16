# Storage Engine 概述

Storage Engine 是 SQLCC 数据库系统中的核心组件之一，负责...

## 主要职责

- 职责1
- 职责2
- 职责3

## 架构设计

组件采用分层架构设计，主要包括...

## 性能特点

- 特点1
- 特点2
- 特点3

---

## 接口与依赖

**核心接口**:
- BufferPool / DiskManager / TableStorage

**关键依赖**:
- types / utils / exception

---

## 验证方式

- 编译验证: `bazel build //src/storage_engine:storage_engine`
- 测试验证: `bazel test //tests/level2_storage_engine/...`
- 覆盖率验证: `bazel coverage //tests/level2_storage_engine/...`

---

## 风险与权衡

- 风险: 高并发下的锁竞争
- 权衡: 通过分片与细粒度锁降低竞争
