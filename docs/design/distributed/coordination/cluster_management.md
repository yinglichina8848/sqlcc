# SQLCC 集群管理设计文档

## 概述

本文档描述SQLCC分布式数据库系统的集群管理架构设计，包括节点发现、领导者选举、配置管理、健康监控等核心功能。该设计基于Raft一致性协议，提供强一致性的集群协调能力。

## 集群管理目标

### 核心目标
- **高可用性**：单点故障不影响整体服务
- **强一致性**：集群状态变化的一致性保证
- **可扩展性**：支持节点动态加入和退出
- **自愈能力**：自动检测和修复故障

### 性能指标
- **领导者选举时间**：< 100ms
- **故障检测延迟**：< 5s
- **配置同步延迟**：< 10s
- **集群可用性**：99.99%

## 集群架构设计

### 总体架构

```
                    ┌─────────────────┐
                    │   Client Apps   │
                    └─────────┬───────┘
                              │
                    ┌─────────▼───────┐
                    │  SQLCC Client   │
                    │   Interface     │
                    └─────────┬───────┘
                              │
┌─────────────────────────────┼─────────────────────────────┐
│                     Cluster Layer                         │
├─────────────────┬───────────┼───────────┬─────────────────┤
│   Load Balancer │           │           │                 │
│     (Optional)  │           │           │                 │
└─────────┬───────┘           │           └─────────┬───────┘
          │                   │                     │
┌─────────▼───────┐   ┌───────▼───────┐   ┌────────▼───────┐
│   SQLCC Node    │   │  SQLCC Node   │   │  SQLCC Node    │
│      #1         │   │      #2       │   │      #3        │
│  (Leader)       │   │  (Follower)   │   │  (Follower)    │
└─────────┬───────┘   └───────────────┘   └─────────┬───────┘
          │                                   │
┌─────────▼───────┐   ┌───────────────────────┼───────┐
│ Cluster Manager │   │ Raft Consensus        │       │
│   & Metadata    │   │ Protocol              │       │
└─────────────────┘   └───────────────────────┼───────┘
                                              │
                                    ┌─────────▼───────┐
                                    │  Distributed    │
                                    │   Configuration │
                                    │   Management    │
                                    └─────────────────┘
```

### 节点类型

#### 1. 领导者节点 (Leader Node)
- **职责**：处理所有客户端请求，维护集群状态
- **唯一性**：集群中同一时间只有一个领导者
- **选举条件**：Raft协议选举产生

#### 2. 跟随者节点 (Follower Node)
- **职责**：处理只读请求，参与领导者选举
- **状态**：被动响应领导者指令
- **数量**：集群中大部分节点

#### 3. 候选者节点 (Candidate Node)
- **职责**：参与领导者选举过程
- **临时状态**：选举过程中的过渡状态
- **超时机制**：定时器触发选举

## 核心组件设计

### 1. 节点管理器 (Node Manager)

#### 节点生命周期
```cpp
class NodeManager {
public:
    // 节点启动
    void StartNode(const NodeConfig& config);
    
    // 节点停止
    void StopNode();
    
    // 加入集群
    Status JoinCluster(const ClusterInfo& cluster);
    
    // 离开集群
    Status LeaveCluster();
    
    // 获取节点状态
    NodeStatus GetNodeStatus() const;
    
    // 更新节点配置
    Status UpdateConfig(const NodeConfig& config);
};
```

#### 节点状态定义
```cpp
enum class NodeState {
    STOPPED,      // 停止状态
    STARTING,     // 启动中
    FOLLOWER,     // 跟随者
    CANDIDATE,    // 候选者
    LEADER,       // 领导者
    SHUTTING_DOWN // 关闭中
};

struct NodeInfo {
    std::string node_id;
    std::string host;
    int port;
    NodeState state;
    uint64_t last_heartbeat;
    double load_factor;
    int64_t capacity;
    std::map<std::string, std::string> metadata;
};
```

### 2. 领导者选举器 (Leader Election)

#### Raft协议实现
```cpp
class RaftConsensus {
private:
    // Raft状态
    NodeState state_;
    uint64_t current_term_;
    std::string voted_for_;
    std::vector<LogEntry> log_;
    uint64_t commit_index_;
    uint64_t last_applied_;
    
    // 选举相关
    std::chrono::milliseconds election_timeout_;
    std::chrono::steady_clock::time_point last_activity_;
    std::map<std::string, int> vote_count_;
    
public:
    // 开始选举
    void StartElection();
    
    // 处理投票请求
    Status HandleVoteRequest(const VoteRequest& request, VoteResponse& response);
    
    // 处理心跳
    Status HandleHeartbeat(const HeartbeatRequest& request, HeartbeatResponse& response);
    
    // 检查选举超时
    bool IsElectionTimeout() const;
    
    // 成为领导者
    void BecomeLeader();
    
    // 成为跟随者
    void BecomeFollower(uint64_t term);
};
```

#### 选举算法
```cpp
void RaftConsensus::StartElection() {
    // 转换为候选者状态
    state_ = NodeState::CANDIDATE;
    current_term_++;
    voted_for_ = current_node_id_;
    vote_count_.clear();
    vote_count_[current_node_id_] = 1;
    
    // 重置选举计时器
    last_activity_ = std::chrono::steady_clock::now();
    
    // 发送投票请求给所有其他节点
    VoteRequest request;
    request.term = current_term_;
    request.candidate_id = current_node_id_;
    request.last_log_index = GetLastLogIndex();
    request.last_log_term = GetLastLogTerm();
    
    BroadcastToPeers(request);
    
    // 如果获得多数票，成为领导者
    if (vote_count_[current_node_id_] > GetMajoritySize()) {
        BecomeLeader();
    }
}
```

### 3. 配置管理器 (Configuration Manager)

#### 分布式配置存储
```cpp
class DistributedConfig {
private:
    std::shared_ptr<RaftConsensus> raft_;
    std::map<std::string, std::string> configs_;
    std::map<std::string, ConfigVersion> version_map_;
    
public:
    // 设置配置
    Status SetConfig(const std::string& key, const std::string& value);
    
    // 获取配置
    Status GetConfig(const std::string& key, std::string& value);
    
    // 删除配置
    Status DeleteConfig(const std::string& key);
    
    // 批量获取配置
    Status GetConfigs(const std::vector<std::string>& keys, 
                     std::map<std::string, std::string>& configs);
    
    // 监听配置变化
    void WatchConfig(const std::string& key, ConfigChangeCallback callback);
};

struct ConfigVersion {
    uint64_t version;
    uint64_t term;
    std::chrono::system_clock::time_point timestamp;
};
```

#### 集群配置
```yaml
cluster:
  name: "sqlcc-cluster-001"
  node_id: "node-001"
  data_dir: "/var/lib/sqlcc/data"
  
  network:
    host: "0.0.0.0"
    port: 5433
    client_port: 5434
    
  raft:
    election_timeout_ms: 150
    heartbeat_interval_ms: 50
    log_compaction_threshold: 1000
    
  replication:
    sync_replication: true
    min_replicas: 2
    replica_lag_threshold: 100MB
    
  monitoring:
    enable_metrics: true
    metrics_port: 9090
    health_check_interval_ms: 5000
```

### 4. 健康监控器 (Health Monitor)

#### 健康检查机制
```cpp
class HealthMonitor {
private:
    std::map<std::string, NodeHealth> node_healths_;
    std::chrono::milliseconds check_interval_;
    std::thread monitor_thread_;
    bool running_;
    
public:
    // 启动监控
    void StartMonitoring();
    
    // 停止监控
    void StopMonitoring();
    
    // 检查节点健康
    NodeHealth CheckNodeHealth(const std::string& node_id);
    
    // 获取所有节点健康状态
    std::map<std::string, NodeHealth> GetAllNodeHealths();
    
    // 添加健康检查回调
    void AddHealthCheckCallback(std::function<void(const std::string&, NodeHealth)> callback);
};

enum class HealthStatus {
    HEALTHY,      // 健康
    UNHEALTHY,    // 不健康
    UNKNOWN,      // 未知
    DEGRADED      // 性能下降
};

struct NodeHealth {
    HealthStatus status;
    double cpu_usage;
    double memory_usage;
    int64_t disk_usage;
    int active_connections;
    double query_latency_p95;
    std::chrono::system_clock::time_point last_check;
    std::vector<std::string> issues;
};
```

#### 性能指标
```cpp
class MetricsCollector {
public:
    // 收集节点指标
    NodeMetrics CollectNodeMetrics();
    
    // 收集集群指标
    ClusterMetrics CollectClusterMetrics();
    
    // 导出指标
    void ExportMetrics(const std::string& format);
    
    // 实时指标
    RealTimeMetrics GetRealTimeMetrics();
};

struct NodeMetrics {
    double cpu_usage;
    double memory_usage;
    int64_t disk_io_read;
    int64_t disk_io_write;
    int active_connections;
    int queries_per_second;
    double query_latency_p50;
    double query_latency_p95;
    double query_latency_p99;
};

struct ClusterMetrics {
    int total_nodes;
    int healthy_nodes;
    int leader_nodes;
    int follower_nodes;
    double average_load;
    int64_t total_queries;
    int64_t total_errors;
    double cluster_availability;
};
```

### 5. 故障检测器 (Failure Detector)

#### Gossip协议实现
```cpp
class GossipFailureDetector {
private:
    std::map<std::string, NodeInfo> member_map_;
    std::map<std::string, std::chrono::steady_clock::time_point> last_seen_;
    std::chrono::milliseconds gossip_interval_;
    std::chrono::milliseconds failure_timeout_;
    
public:
    // 加入集群
    void JoinCluster(const std::string& seed_node);
    
    // 发送Gossip消息
    void SendGossip(const std::string& target_node);
    
    // 处理Gossip消息
    void HandleGossip(const GossipMessage& message);
    
    // 检查节点故障
    std::vector<std::string> DetectFailures();
    
    // 获取活跃节点
    std::vector<std::string> GetAliveNodes();
};

struct GossipMessage {
    std::string from_node;
    std::vector<NodeInfo> members;
    std::map<std::string, std::chrono::steady_clock::time_point> timestamps;
    uint64_t version;
};
```

## 集群操作流程

### 节点启动流程
```
1. 初始化配置
   ↓
2. 启动网络服务
   ↓
3. 加入Gossip网络
   ↓
4. 连接种子节点
   ↓
5. 发现集群成员
   ↓
6. 加入Raft集群
   ↓
7. 同步配置信息
   ↓
8. 开始提供服务
```

### 领导者选举流程
```
1. 检测选举超时
   ↓
2. 转换为候选者
   ↓
3. 增加当前任期
   ↓
4. 给自己投票
   ↓
5. 发送投票请求
   ↓
6. 收集投票结果
   ↓
7. 获得多数票 → 成为领导者
   ↓
8. 其他情况 → 回到跟随者
```

### 故障处理流程
```
1. 检测节点故障
   ↓
2. 标记为不可用
   ↓
3. 触发重新选举
   ↓
4. 数据重新分配
   ↓
5. 客户端重连
   ↓
6. 修复完成后恢复
```

## 配置管理

### 动态配置更新
```cpp
class ConfigUpdateManager {
public:
    // 配置更新流程
    Status UpdateClusterConfig(const ClusterConfig& new_config) {
        // 1. 验证新配置
        if (!ValidateConfig(new_config)) {
            return Status::InvalidArgument("Invalid configuration");
        }
        
        // 2. 通过Raft复制配置
        ConfigChangeLogEntry log_entry;
        log_entry.type = ConfigChangeType::UPDATE_CLUSTER;
        log_entry.new_config = new_config;
        log_entry.version = GenerateVersion();
        
        raft_->AppendLogEntry(log_entry);
        
        // 3. 等待配置生效
        WaitForConfigPropagation(new_config.version);
        
        // 4. 应用新配置
        ApplyConfiguration(new_config);
        
        return Status::OK();
    }
    
private:
    void ApplyConfiguration(const ClusterConfig& config) {
        // 停止旧服务
        StopOldServices();
        
        // 应用新配置
        UpdateNetworkConfig(config.network);
        UpdateReplicationConfig(config.replication);
        UpdateMonitoringConfig(config.monitoring);
        
        // 启动新服务
        StartNewServices();
    }
};
```

### 配置版本控制
```cpp
struct ConfigVersionInfo {
    uint64_t version;
    std::string config_hash;
    std::chrono::system_clock::time_point create_time;
    std::string create_node;
    std::vector<std::string> changes;
    bool is_active;
};

class ConfigVersionManager {
public:
    uint64_t CreateNewVersion(const ClusterConfig& config);
    Status ActivateVersion(uint64_t version);
    Status RollbackVersion(uint64_t target_version);
    std::vector<ConfigVersionInfo> GetVersionHistory();
};
```

## 数据一致性保证

### Raft日志复制
```cpp
class RaftLogManager {
public:
    // 追加日志条目
    Status AppendLogEntry(const LogEntry& entry);
    
    // 同步日志到跟随者
    void ReplicateLog(const std::string& target_node);
    
    // 处理日志复制响应
    Status HandleAppendEntriesResponse(const AppendEntriesResponse& response);
    
    // 提交日志条目
    Status CommitLogEntries(uint64_t commit_index);
    
    // 获取日志条目
    LogEntry GetLogEntry(uint64_t index);
    
    // 获取最后一条日志
    LogEntry GetLastLogEntry();
};

struct LogEntry {
    uint64_t index;
    uint64_t term;
    LogEntryType type;
    std::vector<uint8_t> data;
    std::chrono::system_clock::time_point timestamp;
};

enum class LogEntryType {
    CONFIG_CHANGE,
    MEMBER_CHANGE,
    DATA_OPERATION,
    SNAPSHOT
};
```

### 快照机制
```cpp
class SnapshotManager {
public:
    // 创建快照
    Status CreateSnapshot(const std::string& snapshot_path);
    
    // 加载快照
    Status LoadSnapshot(const std::string& snapshot_path);
    
    // 定期快照
    void StartPeriodicSnapshot(std::chrono::milliseconds interval);
    
    // 清理旧快照
    void CleanupOldSnapshots(int keep_count);
    
private:
    std::string GenerateSnapshotPath();
    Status CompressSnapshot(const std::string& path);
    Status VerifySnapshot(const std::string& path);
};
```

## 监控和告警

### 监控指标
```yaml
metrics:
  cluster:
    - node_count
    - leader_count
    - follower_count
    - cluster_availability
    - config_version
    
  node:
    - cpu_usage
    - memory_usage
    - disk_usage
    - network_connections
    - query_latency
    - error_rate
    
  raft:
    - current_term
    - log_index
    - commit_index
    - applied_index
    - election_count
    - heartbeat_count
    
  replication:
    - replica_count
    - sync_lag
    - replication_rate
    - error_count
```

### 告警规则
```yaml
alerts:
  - name: "HighErrorRate"
    condition: "error_rate > 0.01"
    severity: "critical"
    actions: ["email", "slack"]
    
  - name: "LeaderDown"
    condition: "leader_count == 0"
    severity: "critical"
    actions: ["email", "pagerduty"]
    
  - name: "HighLatency"
    condition: "query_latency_p95 > 1000ms"
    severity: "warning"
    actions: ["slack"]
    
  - name: "DiskSpaceLow"
    condition: "disk_usage > 0.8"
    severity: "warning"
    actions: ["email"]
    
  - name: "NodeUnreachable"
    condition: "healthy_nodes < total_nodes * 0.5"
    severity: "critical"
    actions: ["email", "pagerduty"]
```

## 测试策略

### 功能测试
- 节点启动和停止
- 领导者选举
- 故障检测和恢复
- 配置更新
- 数据一致性

### 性能测试
- 选举延迟测试
- 故障恢复时间
- 配置同步延迟
- 资源使用情况

### 压力测试
- 大量节点并发启动
- 网络分区测试
- 极端故障场景
- 长时间运行稳定性

### 混沌工程
- 随机节点故障
- 网络延迟注入
- 资源耗尽测试
- 配置错误处理

## 部署和运维

### 部署配置
```bash
# 集群初始化
sqlcc cluster init \
  --cluster-name sqlcc-cluster-001 \
  --node-id node-001 \
  --host 192.168.1.100 \
  --port 5433

# 节点启动
sqlcc node start \
  --config /etc/sqlcc/cluster.yaml \
  --data-dir /var/lib/sqlcc/data \
  --log-level INFO

# 加入集群
sqlcc node join \
  --seed-node 192.168.1.100:5433 \
  --node-id node-002
```

### 运维命令
```bash
# 查看集群状态
sqlcc cluster status

# 查看节点列表
sqlcc node list

# 查看领导者信息
sqlcc node leader

# 手动故障转移
sqlcc cluster failover

# 节点维护模式
sqlcc node maintenance enable

# 配置更新
sqlcc cluster config update cluster.yaml

# 备份配置
sqlcc cluster config backup backup.yaml
```

### 故障处理
1. **节点故障**
   - 自动检测和隔离
   - 数据重新分配
   - 客户端重定向

2. **网络分区**
   - 少数派自动停止
   - 恢复后数据同步
   - 一致性验证

3. **配置错误**
   - 配置验证检查
   - 回滚机制
   - 审计日志记录

## 总结

SQLCC集群管理设计基于成熟的Raft协议和Gossip机制，提供了高可用、强一致性的集群协调能力。通过模块化的组件设计和完整的监控告警系统，确保集群的稳定运行和快速故障恢复。

关键设计特点：
1. **基于Raft的一致性保证**
2. **Gossip协议的高效节点发现**
3. **动态配置管理**
4. **全面的健康监控**
5. **自动化故障处理**

这一设计为SQLCC分布式数据库提供了坚实的集群管理基础，确保系统的可靠性和可扩展性。
