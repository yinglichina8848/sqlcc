# SQLCC 网络协议规范

## 协议概述

本文档定义SQLCC分布式数据库系统的网络通信协议规范。该协议用于客户端与服务器、集群节点间的通信，支持SQL查询执行、事务处理、集群管理等操作。

## 协议基础

### 传输层
- **主要传输**：TCP/IP（端口：5433）
- **备用传输**：Unix Domain Socket（/var/run/sqlcc.sock）
- **版本支持**：IPv4, IPv6

### 协议特性
- **请求-响应模式**：同步RPC调用
- **异步通知**：服务器推送机制
- **连接复用**：支持多路复用
- **安全传输**：TLS 1.3加密支持

## 协议格式

### 消息结构
```
| Length (4 bytes) | Type (1 byte) | Version (1 byte) | Sequence (4 bytes) | Payload (Variable) |
```

#### 字段说明
- **Length**: 消息总长度（包括头部）
- **Type**: 消息类型（1-10：请求，100-110：响应，200+：通知）
- **Version**: 协议版本（当前：0x01）
- **Sequence**: 请求序列号，用于响应匹配
- **Payload**: 消息负载，JSON或二进制格式

### 消息类型定义

#### 请求消息（客户端→服务器）
| 类型值 | 消息类型 | 说明 |
|--------|----------|------|
| 0x01 | CONNECT | 建立连接 |
| 0x02 | DISCONNECT | 断开连接 |
| 0x03 | EXECUTE_SQL | 执行SQL语句 |
| 0x04 | PREPARE_SQL | 预编译SQL |
| 0x05 | EXECUTE_PREPARED | 执行预编译语句 |
| 0x06 | BEGIN_TRANSACTION | 开始事务 |
| 0x07 | COMMIT_TRANSACTION | 提交事务 |
| 0x08 | ROLLBACK_TRANSACTION | 回滚事务 |
| 0x09 | GET_METADATA | 获取元数据 |
| 0x0A | HEARTBEAT | 心跳检测 |

#### 响应消息（服务器→客户端）
| 类型值 | 消息类型 | 说明 |
|--------|----------|------|
| 0x64 | CONNECT_OK | 连接成功 |
| 0x65 | CONNECT_ERROR | 连接失败 |
| 0x66 | EXECUTE_RESULT | SQL执行结果 |
| 0x67 | EXECUTE_ERROR | 执行错误 |
| 0x68 | PREPARED_OK | 预编译成功 |
| 0x69 | TRANSACTION_STARTED | 事务开始 |
| 0x6A | TRANSACTION_COMMITTED | 事务提交成功 |
| 0x6B | TRANSACTION_ROLLED_BACK | 事务回滚成功 |
| 0x6C | METADATA_RESULT | 元数据结果 |
| 0x6D | HEARTBEAT_RESPONSE | 心跳响应 |

#### 通知消息（服务器→客户端）
| 类型值 | 消息类型 | 说明 |
|--------|----------|------|
| 0xC8 | ERROR_NOTIFICATION | 错误通知 |
| 0xC9 | WARNING_NOTIFICATION | 警告通知 |
| 0xCA | TRANSACTION_COMPLETED | 事务完成通知 |
| 0xCB | CLUSTER_EVENT | 集群事件通知 |

## 连接建立

### 连接握手
```json
{
  "type": "CONNECT",
  "version": 1,
  "client_info": {
    "name": "sqlcc-cli",
    "version": "1.0.0",
    "capabilities": ["transaction", "prepared_statement", "metadata"]
  },
  "auth": {
    "method": "password",
    "username": "user",
    "password_hash": "hashed_password"
  }
}
```

### 响应格式
```json
{
  "type": "CONNECT_OK",
  "server_info": {
    "name": "sqlcc-server",
    "version": "1.0.0",
    "cluster_id": "sqlcc-cluster-001",
    "node_id": "node-001",
    "capabilities": ["transaction", "prepared_statement", "cluster"]
  },
  "session": {
    "session_id": "session-12345",
    "timeout": 3600
  }
}
```

## SQL执行协议

### 简单SQL执行
```json
{
  "type": "EXECUTE_SQL",
  "sequence": 1,
  "sql": "SELECT * FROM users WHERE id = 123",
  "parameters": {},
  "options": {
    "timeout": 30,
    "fetch_size": 1000
  }
}
```

### 响应格式
```json
{
  "type": "EXECUTE_RESULT",
  "sequence": 1,
  "status": "success",
  "result": {
    "rows_affected": 0,
    "execution_time": 15.5,
    "data": [
      {
        "id": 123,
        "name": "John Doe",
        "email": "john@example.com"
      }
    ],
    "columns": [
      {"name": "id", "type": "INTEGER"},
      {"name": "name", "type": "VARCHAR"},
      {"name": "email", "type": "VARCHAR"}
    ]
  }
}
```

### 错误响应
```json
{
  "type": "EXECUTE_ERROR",
  "sequence": 1,
  "error": {
    "code": "SQL_SYNTAX_ERROR",
    "message": "Syntax error at 'FROM'",
    "details": "Unexpected token 'FROM' at position 14",
    "line": 1,
    "column": 14
  }
}
```

## 事务协议

### 开始事务
```json
{
  "type": "BEGIN_TRANSACTION",
  "sequence": 2,
  "options": {
    "isolation_level": "READ_COMMITTED",
    "read_only": false,
    "timeout": 60
  }
}
```

### 提交事务
```json
{
  "type": "COMMIT_TRANSACTION",
  "sequence": 3,
  "transaction_id": "txn-abc123"
}
```

## 预编译语句协议

### 预编译SQL
```json
{
  "type": "PREPARE_SQL",
  "sequence": 4,
  "sql": "SELECT * FROM users WHERE id = ? AND name = ?",
  "parameter_types": ["INTEGER", "VARCHAR"]
}
```

### 执行预编译语句
```json
{
  "type": "EXECUTE_PREPARED",
  "sequence": 5,
  "statement_id": "stmt-xyz789",
  "parameters": [123, "John Doe"]
}
```

## 元数据协议

### 获取表结构
```json
{
  "type": "GET_METADATA",
  "sequence": 6,
  "request": {
    "type": "table_schema",
    "database": "test_db",
    "table": "users"
  }
}
```

### 响应格式
```json
{
  "type": "METADATA_RESULT",
  "sequence": 6,
  "result": {
    "database": "test_db",
    "table": "users",
    "columns": [
      {
        "name": "id",
        "type": "INTEGER",
        "nullable": false,
        "default_value": null,
        "primary_key": true
      },
      {
        "name": "name",
        "type": "VARCHAR",
        "length": 255,
        "nullable": false,
        "default_value": null
      }
    ],
    "indexes": [
      {
        "name": "idx_users_id",
        "type": "PRIMARY",
        "columns": ["id"]
      }
    ]
  }
}
```

## 集群管理协议

### 节点状态更新（集群间通信）
```json
{
  "type": "CLUSTER_EVENT",
  "sequence": 7,
  "event": {
    "type": "NODE_STATUS_CHANGE",
    "node_id": "node-002",
    "status": "ONLINE",
    "load": 0.65,
    "capacity": 1000
  }
}
```

### 数据分片路由查询
```json
{
  "type": "GET_METADATA",
  "sequence": 8,
  "request": {
    "type": "shard_routing",
    "table": "users",
    "key": "123"
  }
}
```

## 心跳和健康检查

### 心跳消息
```json
{
  "type": "HEARTBEAT",
  "sequence": 9,
  "timestamp": 1609459200000
}
```

### 心跳响应
```json
{
  "type": "HEARTBEAT_RESPONSE",
  "sequence": 9,
  "timestamp": 1609459200000,
  "server_time": 1609459200001
}
```

## 错误码定义

### 系统错误码
| 错误码 | 说明 | HTTP对应码 |
|--------|------|------------|
| 0x0001 | CONNECTION_FAILED | 503 |
| 0x0002 | AUTHENTICATION_FAILED | 401 |
| 0x0003 | PERMISSION_DENIED | 403 |
| 0x0004 | RESOURCE_NOT_FOUND | 404 |
| 0x0005 | INVALID_REQUEST | 400 |
| 0x0006 | TIMEOUT | 408 |
| 0x0007 | SERVER_OVERLOADED | 503 |

### SQL执行错误码
| 错误码 | 说明 |
|--------|------|
| 0x1001 | SQL_SYNTAX_ERROR |
| 0x1002 | TABLE_NOT_FOUND |
| 0x1003 | COLUMN_NOT_FOUND |
| 0x1004 | CONSTRAINT_VIOLATION |
| 0x1005 | TRANSACTION_CONFLICT |
| 0x1006 | DEADLOCK_DETECTED |
| 0x1007 | QUERY_TIMEOUT |

### 分布式错误码
| 错误码 | 说明 |
|--------|------|
| 0x2001 | CLUSTER_NOT_AVAILABLE |
| 0x2002 | NODE_NOT_FOUND |
| 0x2003 | SHARD_NOT_FOUND |
| 0x2004 | CONSENSUS_TIMEOUT |
| 0x2005 | DATA_INCONSISTENCY |

## 安全考虑

### 认证机制
- **密码认证**：SHA-256哈希 + 盐值
- **证书认证**：X.509客户端证书
- **令牌认证**：JWT令牌支持
- **LDAP集成**：企业级认证

### 加密传输
- **TLS 1.3**：强制加密传输
- **证书验证**：服务器证书验证
- **证书固定**：防止中间人攻击

### 访问控制
- **角色基权限**：RBAC权限模型
- **行级安全**：RLS策略支持
- **审计日志**：完整的操作审计

## 性能优化

### 连接池
- **最大连接数**：可配置连接池大小
- **连接复用**：避免频繁建立连接
- **负载均衡**：智能连接分配

### 请求优化
- **批处理**：支持批量请求
- **流式传输**：大数据集流式返回
- **压缩**：gzip压缩支持

### 缓存
- **查询计划缓存**：预编译语句缓存
- **元数据缓存**：表结构信息缓存
- **分片路由缓存**：路由信息缓存

## 扩展性

### 协议版本控制
- **向后兼容**：支持旧版本客户端
- **功能协商**：运行时能力检测
- **平滑升级**：零停机协议升级

### 自定义扩展
- **插件接口**：支持自定义消息类型
- **事件系统**：客户端事件监听
- **监控指标**：协议级性能指标

## 实现指南

### 客户端实现
1. 建立TCP连接
2. 发送CONNECT请求
3. 等待CONNECT_OK响应
4. 发送业务请求
5. 处理响应和错误
6. 发送DISCONNECT请求

### 服务器实现
1. 监听端口
2. 接受连接
3. 处理认证
4. 请求路由
5. 响应生成
6. 连接管理

### 集群节点通信
1. 节点注册
2. 心跳维护
3. 状态同步
4. 故障检测
5. 重新选举

## 测试规范

### 连接测试
- 连接建立和断开
- 认证流程
- 并发连接

### 功能测试
- SQL执行
- 事务处理
- 错误处理

### 性能测试
- 并发性能
- 延迟测试
- 吞吐量测试

### 容错测试
- 网络中断
- 节点故障
- 数据恢复

## 部署考虑

### 网络要求
- **端口开放**：5433端口对外访问
- **防火墙**：合理的防火墙规则
- **负载均衡**：HAProxy/Nginx配置

### 监控要求
- **连接监控**：连接数和状态
- **性能监控**：延迟和吞吐量
- **错误监控**：错误率和类型

### 安全加固
- **证书管理**：定期更新证书
- **访问控制**：严格的权限管理
- **审计日志**：完整的审计记录

这一协议规范为SQLCC分布式系统提供了完整的网络通信基础，确保了系统的可靠性、安全性和性能。
