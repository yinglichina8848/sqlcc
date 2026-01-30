/**
 * WHY: 为什么需要专门的连接状态机？
 *
 * 数据库连接状态管理复杂度极高，传统方案存在诸多技术挑战：
 * - 状态转换逻辑复杂：连接的认证、加密、关闭等状态间的转换规则繁琐
 * - 并发安全问题严重：多线程环境下状态变更可能导致竞态条件
 * - 错误恢复机制不完善：网络异常时的自动重连和状态恢复处理困难
 * - 超时管理混乱：连接超时、重连间隔、活动检测等时间管理分散
 * - 状态监控缺失：连接状态变化的实时监控和统计信息不足
 * - 业务逻辑耦合：状态机逻辑与业务处理逻辑紧密耦合难以维护
 * - 扩展性限制：新状态、新事件的添加影响现有状态机逻辑
 *
 * 连接状态机的核心价值：
 * 1. 状态转换安全性：严格验证状态转换的有效性和合法性
 * 2. 线程安全保证：多线程环境下状态变更的原子性和一致性
 * 3. 自动重连机制：网络异常后的智能重连和恢复策略
 * 4. 超时管理统一：连接生命周期的超时检测和处理机制
 * 5. 状态监控透明：连接状态变化的实时监控和统计
 * 6. 业务逻辑隔离：状态管理与业务处理的清晰分离
 * 7. 扩展性架构：新状态和新事件的灵活添加机制
 *
 * 🏗️ 设计模式：状态机模式(State Machine Pattern) + 观察者模式(Observer Pattern) + 策略模式(Strategy Pattern)
 *
 * 连接状态机作为状态机模式的应用：
 * - 状态封装清晰：每个连接状态被封装为独立的处理逻辑
 * - 事件驱动转换：外部事件触发状态间的转换和处理
 * - 状态行为隔离：不同状态下的行为逻辑完全隔离
 * - 转换规则明确：状态转换的规则和条件清晰定义
 * - 状态历史可追踪：状态变化的历史记录和回溯
 * - 异常状态处理：错误状态的统一检测和恢复
 *
 * SOLID原则体现：
 * - 单一职责：专门负责数据库连接状态的管理和转换
 * - 开闭原则：新状态通过策略模式实现扩展无需修改核心
 * - 里氏替换：所有状态机实现都可以互相替换
 * - 接口隔离：状态机接口精确定义状态管理契约
 * - 依赖倒置：依赖抽象的状态转换和事件处理接口
 *
 * WHAT: 连接状态机系统 - 数据库连接生命周期的完整状态管理框架
 *
 * 核心功能：
 * - 状态转换管理：严格控制连接状态间的转换逻辑和验证
 * - 线程安全保护：多线程环境下状态变更的原子性和同步
 * - 自动重连机制：网络异常后的智能重连和恢复策略
 * - 超时管理控制：连接生命周期的各种超时检测和处理
 * - 状态监控统计：连接状态变化的实时监控和统计信息
 * - 事件回调通知：状态变化时的外部通知和回调机制
 * - 配置参数管理：状态机行为的灵活配置和参数调整
 *
 * 系统组件：
 * - ConnectionStateMachine：核心状态机类，提供完整的连接状态管理
 * - StateTransitionValidator：状态转换验证器，验证状态转换的合法性
 * - TimeoutManager：超时管理器，管理各种超时检测和处理
 * - ReconnectStrategy：重连策略，实现不同的重连算法和策略
 * - KeepAliveMonitor：保活监控器，监控连接的活跃状态
 * - StateChangeObserver：状态变更观察者，观察状态变化并通知
 * - ConfigurationManager：配置管理器，管理状态机配置参数
 *
 * 状态转换流程：
 * - 初始状态：连接创建时的初始未连接状态
 * - 连接建立：发起连接请求并等待连接建立
 * - 认证处理：连接建立后进行用户身份认证
 * - 密钥交换：认证成功后进行加密密钥交换
 * - 正常通信：认证和加密完成后进入正常通信状态
 * - 错误处理：任何阶段出现错误进入错误状态
 * - 重连尝试：错误状态下根据策略进行重连尝试
 * - 连接关闭：主动或异常情况下关闭连接
 *
 * 状态枚举定义：
 * - DISCONNECTED：未连接状态，初始状态或连接关闭后的状态
 * - CONNECTING：正在连接状态，发起连接请求等待响应
 * - CONNECTED：已连接状态，TCP连接已建立但未认证
 * - AUTHENTICATING：认证中状态，正在进行用户身份验证
 * - AUTHENTICATED：已认证状态，身份验证通过但未加密
 * - KEY_EXCHANGING：密钥交换状态，正在进行加密密钥协商
 * - ENCRYPTED：已加密状态，加密通信已建立可以安全通信
 * - CLOSING：正在关闭状态，正在执行连接关闭流程
 * - CLOSED：已关闭状态，连接已完全关闭
 * - CONNECTION_ERROR：连接错误状态，连接出现不可恢复错误
 *
 * 事件驱动机制：
 * - CONNECT_REQUEST：连接请求事件，发起新的连接尝试
 * - CONNECT_SUCCESS：连接成功事件，TCP连接建立成功
 * - CONNECT_FAILURE：连接失败事件，TCP连接建立失败
 * - AUTHENTICATE_REQUEST：认证请求事件，开始用户身份验证
 * - AUTHENTICATE_SUCCESS：认证成功事件，身份验证通过
 * - AUTHENTICATE_FAILURE：认证失败事件，身份验证失败
 * - KEY_EXCHANGE_REQUEST：密钥交换请求事件，开始密钥协商
 * - KEY_EXCHANGE_SUCCESS：密钥交换成功事件，密钥协商完成
 * - KEY_EXCHANGE_FAILURE：密钥交换失败事件，密钥协商失败
 * - DISCONNECT_REQUEST：断开请求事件，主动断开连接
 * - DISCONNECT_SUCCESS：断开成功事件，连接正常关闭
 * - TIMEOUT：超时事件，连接或操作超时
 * - ERROR：错误事件，发生不可恢复的错误
 * - RECONNECT：重连事件，触发重连尝试
 *
 * 超时管理机制：
 * - 连接超时：TCP连接建立的最大等待时间
 * - 认证超时：用户身份验证的最大等待时间
 * - 密钥交换超时：加密密钥交换的最大等待时间
 * - 重连间隔：重连尝试之间的最小间隔时间
 * - 保活间隔：发送保活消息的间隔时间
 * - 空闲超时：连接空闲状态的最大持续时间
 * - 操作超时：单个操作执行的最大时间限制
 *
 * 重连策略实现：
 * - 指数退避：重连间隔呈指数增长避免网络风暴
 * - 最大重试：限制最大重连次数避免无限重连
 * - 随机抖动：在重连间隔基础上添加随机抖动
 * - 条件重连：根据错误类型决定是否进行重连
 * - 渐进延迟：重连间隔逐渐增加减轻服务器压力
 * - 智能重连：根据网络状况动态调整重连策略
 *
 * 保活机制实现：
 * - 心跳包发送：定期发送心跳包检测连接活跃状态
 * - 响应超时检测：心跳响应超时的连接判定为异常
 * - 自动重连触发：保活失败时自动触发重连机制
 * - 保活参数配置：心跳间隔、超时时间等参数可配置
 * - 保活统计监控：保活成功率和响应时间的统计
 * - 异常检测告警：保活异常时的告警和通知机制
 *
 * 监控和统计：
 * - 状态转换统计：各状态转换次数和成功率的统计
 * - 连接时长统计：连接建立到关闭的总时长的统计
 * - 重连次数统计：重连尝试次数和成功率的统计
 * - 超时事件统计：各种超时事件的发生次数统计
 * - 错误状态统计：错误状态的分类和数量统计
 * - 性能指标监控：状态转换延迟、响应时间等性能指标
 *
 * 接口设计：
 * - 状态查询接口：获取当前状态和状态历史查询
 * - 状态控制接口：主动触发状态转换和状态重置
 * - 配置管理接口：状态机参数的配置和调整接口
 * - 监控统计接口：状态机监控数据的获取接口
 * - 回调注册接口：状态变更回调函数的注册接口
 * - 扩展策略接口：自定义状态转换策略的接口
 * - 调试诊断接口：状态机调试和诊断信息的接口
 *
 * HOW: 连接状态机系统的实现机制
 *
 * 状态机模式实现：
 * 1. 状态接口定义：定义统一的连接状态接口和行为契约
 * 2. 具体状态实现：为每个枚举状态实现对应的状态类
 * 3. 状态转换表：定义状态间转换的规则和条件矩阵
 * 4. 事件处理机制：事件触发器和事件分发器的实现
 * 5. 状态上下文：维护当前状态和状态转换历史
 * 6. 状态验证逻辑：状态转换前的合法性验证机制
 * 7. 状态持久化：状态信息的序列化和持久化存储
 *
 * 观察者模式支撑：
 * 1. 观察者接口：定义状态变更观察者的统一接口
 * 2. 主题对象实现：状态机作为主题对象管理观察者列表
 * 3. 通知机制实现：状态变更时通知所有注册的观察者
 * 4. 回调函数管理：观察者回调函数的注册和注销管理
 * 5. 异步通知处理：状态变更通知的异步处理机制
 * 6. 通知过滤机制：根据观察者兴趣过滤通知内容
 * 7. 观察者生命周期：观察者对象的生命周期管理
 *
 * 策略模式支撑：
 * 1. 重连策略接口：定义重连算法的统一接口
 * 2. 具体策略实现：指数退避、线性退避等具体策略
 * 3. 策略选择机制：运行时动态选择重连策略
 * 4. 策略配置接口：策略参数的动态配置接口
 * 5. 策略切换机制：运行时切换重连策略的能力
 * 6. 策略扩展框架：新重连策略的插件化扩展
 * 7. 策略效果评估：不同策略效果的对比和评估
 *
 * 状态转换实现：
 * 1. 事件接收处理：外部事件信号的接收和初步处理
 * 2. 状态验证检查：当前状态下事件处理的合法性验证
 * 3. 转换条件判断：目标状态转换的条件和约束检查
 * 4. 状态切换执行：原子性的状态切换和上下文更新
 * 5. 副作用处理：状态切换引起的副作用和清理操作
 * 6. 通知广播发送：状态变更通知的广播和异步处理
 * 7. 日志记录归档：状态转换过程的详细日志记录
 *
 * 线程安全实现：
 * 1. 互斥锁保护：状态变量的互斥锁原子性保护
 * 2. 条件变量同步：状态变更的条件变量通知机制
 * 3. 读写锁优化：状态查询的读写锁性能优化
 * 4. 原子操作应用：状态计数器的原子操作更新
 * 5. 内存屏障保证：多核环境下内存一致性保证
 * 6. 死锁预防机制：锁顺序规范和死锁检测机制
 * 7. 锁粒度控制：最小化锁的持有范围和时间
 *
 * 超时管理实现：
 * 1. 定时器管理：各种超时的定时器创建和管理
 * 2. 时间轮算法：高效的定时器过期检测算法
 * 3. 超时回调处理：超时事件的回调函数处理机制
 * 4. 定时器取消机制：超时事件取消和清理机制
 * 5. 时间精度控制：高精度时间测量和控制机制
 * 6. 超时统计收集：超时事件的统计和分析收集
 * 7. 动态调整机制：根据负载动态调整超时参数
 *
 * 重连策略实现：
 * 1. 策略接口抽象：重连策略的统一抽象接口
 * 2. 指数退避算法：重连间隔指数增长的实现
 * 3. 随机抖动添加：避免重连风暴的随机化处理
 * 4. 最大重试限制：防止无限重连的次数限制
 * 5. 条件判断逻辑：基于错误类型的重连决策
 * 6. 策略切换机制：运行时重连策略的动态切换
 * 7. 策略效果监控：重连成功率和效率的监控统计
 *
 * 保活机制实现：
 * 1. 心跳包构造：心跳消息的构造和序列化
 * 2. 定时发送机制：心跳包的定时发送调度
 * 3. 响应等待处理：心跳响应的等待和超时检测
 * 4. 连接活性判断：基于心跳的连接活性状态判断
 * 5. 异常触发重连：保活失败时的重连触发机制
 * 6. 保活参数调优：心跳间隔和超时参数的动态调优
 * 7. 保活统计分析：保活成功率和延迟的统计分析
 *
 * 监控统计实现：
 * 1. 指标收集接口：各种状态机指标的收集接口
 * 2. 统计数据存储：统计数据的存储和聚合处理
 * 3. 实时监控面板：状态机状态的实时监控界面
 * 4. 历史数据保存：状态机运行历史数据的保存
 * 5. 告警阈值设置：关键指标的告警阈值配置
 * 6. 性能分析工具：状态机性能的分析和优化工具
 * 7. 可视化展示：状态机状态和统计的可视化展示
 *
 * 配置管理实现：
 * 1. 配置参数定义：状态机所有配置参数的定义
 * 2. 配置验证逻辑：配置参数的有效性验证
 * 3. 动态配置更新：运行时配置参数的动态更新
 * 4. 配置持久化存储：配置参数的持久化保存
 * 5. 配置版本管理：配置变更的版本控制和回滚
 * 6. 配置热更新：无需重启的状态机配置热更新
 * 7. 配置模板支持：不同场景的配置模板支持
 *
 * 调试诊断实现：
 * 1. 状态转换日志：详细的状态转换过程日志记录
 * 2. 事件触发追踪：外部事件的接收和处理追踪
 * 3. 超时事件调试：超时事件的触发和处理调试信息
 * 4. 重连过程记录：重连尝试的详细过程记录
 * 5. 保活状态监控：保活机制的状态和统计监控
 * 6. 性能分析报告：状态机性能的详细分析报告
 * 7. 故障排查工具：状态机故障的自动化排查工具
 *
 * 扩展性设计：
 * - 插件化状态：支持自定义连接状态的插件化扩展
 * - 事件扩展机制：支持自定义事件的注册和处理
 * - 策略插件系统：重连策略的插件化架构
 * - 监控扩展接口：自定义监控指标的扩展接口
 * - 配置扩展机制：配置参数的动态扩展机制
 * - 回调扩展框架：回调函数的扩展和定制框架
 *
 * 调试和诊断：
 * - 状态机状态图：连接状态机的可视化状态图展示
 * - 转换序列记录：状态转换的时间序列记录和分析
 * - 性能瓶颈分析：状态机处理的性能瓶颈识别
 * - 错误模式识别：常见错误模式的自动识别和分类
 * - 自动化测试工具：状态机逻辑的自动化测试框架
 * - 故障注入测试：状态机容错能力的故障注入测试
 * - 运行时诊断：状态机的运行时健康检查和诊断
 */

#pragma once

#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <string>

namespace sqlcc {
namespace network {

/**
 * @brief 连接状态枚举
 *
 * 严格定义连接状态机的所有可能状态
 */
enum ConnectionState {
    DISCONNECTED = 0,       ///< 未连接
    CONNECTING = 1,         ///< 正在连接
    CONNECTED = 2,          ///< 已连接但未认证
    AUTHENTICATING = 3,     ///< 正在认证
    AUTHENTICATED = 4,      ///< 已认证可以正常通信
    KEY_EXCHANGING = 5,     ///< 正在进行密钥交换
    ENCRYPTED = 6,          ///< 已加密可以安全通信
    CLOSING = 7,           ///< 正在关闭
    CLOSED = 8,            ///< 已关闭
    CONNECTION_ERROR = 9   ///< 连接错误状态
};

/**
 * @brief 连接状态机类
 *
 * 严格验证连接状态转换安全性，提供重连、超时和保活机制
 */
class ConnectionStateMachine {
public:
    /**
     * @brief 构造函数
     */
    ConnectionStateMachine();

    /**
     * @brief 析构函数
     */
    ~ConnectionStateMachine() = default;

    /**
     * @brief 获取当前状态
     * @return 当前连接状态
     */
    ConnectionState GetCurrentState() const;

    /**
     * @brief 检查是否处于指定状态
     * @param state 要检查的状态
     * @return 是否处于该状态
     */
    bool IsInState(ConnectionState state) const;

    /**
     * @brief 获取状态名称
     * @param state 状态枚举值
     * @return 状态的字符串名称
     */
    std::string GetStateName(ConnectionState state) const;

    /**
     * @brief 转换到新状态
     * @param new_state 目标状态
     * @return 是否转换成功
     */
    bool TransitionTo(ConnectionState new_state);

    /**
     * @brief 检查是否可以转换到指定状态
     * @param new_state 目标状态
     * @return 是否可以转换
     */
    bool CanTransitionTo(ConnectionState new_state) const;

    // 便捷状态检查方法
    bool IsDisconnected() const { return state_ == DISCONNECTED; }
    bool IsConnecting() const { return state_ == CONNECTING; }
    bool IsConnected() const { return state_ == CONNECTED || state_ == AUTHENTICATED || state_ == ENCRYPTED; }
    bool IsAuthenticated() const { return state_ == AUTHENTICATED || state_ == ENCRYPTED; }
    bool IsEncrypted() const { return state_ == ENCRYPTED; }
    bool IsClosed() const { return state_ == CLOSED || state_ == CLOSING; }
    bool IsError() const { return state_ == CONNECTION_ERROR; }

    // 允许的操作检查
    bool CanConnect() const;
    bool CanAuthenticate() const;
    bool CanSendQuery() const;
    bool CanEncrypt() const;
    bool CanClose() const;

    /**
     * @brief 重置状态机
     */
    void Reset();

    /**
     * @brief 强制设置为错误状态
     */
    void ForceError();

    /**
     * @brief 设置状态变更回调
     * @param callback 状态变更回调函数
     */
    using StateChangeCallback = std::function<void(ConnectionState old_state, ConnectionState new_state)>;
    void SetStateChangeCallback(StateChangeCallback callback);

    /**
     * @brief 设置超时和重连参数
     * @param connect_timeout 连接超时时间
     * @param reconnect_delay 重连延迟时间
     * @param keep_alive_interval 保活间隔时间
     */
    void SetTimeouts(std::chrono::milliseconds connect_timeout = std::chrono::seconds(30),
                    std::chrono::milliseconds reconnect_delay = std::chrono::seconds(5),
                    std::chrono::milliseconds keep_alive_interval = std::chrono::seconds(60));

    /**
     * @brief 启动连接计时器
     */
    void StartConnectionTimer();

    /**
     * @brief 停止连接计时器
     */
    void StopConnectionTimer();

    /**
     * @brief 检查连接是否超时
     * @return 是否超时
     */
    bool IsConnectionTimedOut() const;

    /**
     * @brief 重置连接计时器
     */
    void ResetConnectionTimer();

    /**
     * @brief 启用自动重连
     * @param enabled 是否启用
     * @param max_retry_attempts 最大重试次数
     */
    void EnableAutoReconnect(bool enabled, int max_retry_attempts = 3);

    /**
     * @brief 检查是否启用自动重连
     * @return 是否启用
     */
    bool IsAutoReconnectEnabled() const;

    /**
     * @brief 获取最大重试次数
     * @return 最大重试次数
     */
    int GetMaxRetryAttempts() const;

    /**
     * @brief 获取当前重试次数
     * @return 当前重试次数
     */
    int GetCurrentRetryCount() const;

    /**
     * @brief 增加重试次数
     */
    void IncrementRetryCount();

    /**
     * @brief 重置重试次数
     */
    void ResetRetryCount();

    /**
     * @brief 检查是否应该尝试重连
     * @return 是否应该重连
     */
    bool ShouldAttemptReconnect() const;

    /**
     * @brief 获取重连延迟时间
     * @return 重连延迟时间
     */
    std::chrono::milliseconds GetReconnectDelay() const;

    /**
     * @brief 启用保活机制
     * @param enabled 是否启用
     */
    void EnableKeepAlive(bool enabled);

    /**
     * @brief 检查是否启用保活机制
     * @return 是否启用
     */
    bool IsKeepAliveEnabled() const;

    /**
     * @brief 更新最后活动时间
     */
    void UpdateLastActivity();

    /**
     * @brief 检查保活是否超时
     * @return 是否超时
     */
    bool IsKeepAliveTimeout() const;

private:
    ConnectionState state_;                           ///< 当前状态
    mutable std::mutex mutex_;                        ///< 线程安全锁
    StateChangeCallback state_change_callback_;       ///< 状态变更回调

    // 超时和重连相关成员变量
    std::chrono::milliseconds connect_timeout_;       ///< 连接超时时间
    std::chrono::milliseconds reconnect_delay_;       ///< 重连延迟时间
    std::chrono::milliseconds keep_alive_interval_;   ///< 保活间隔时间

    std::chrono::steady_clock::time_point connection_start_time_;  ///< 连接开始时间
    std::chrono::steady_clock::time_point last_activity_time_;     ///< 最后活动时间
    bool connection_timer_active_;                      ///< 连接计时器是否激活
    bool auto_reconnect_enabled_;                       ///< 是否启用自动重连
    bool keep_alive_enabled_;                           ///< 是否启用保活
    int max_retry_attempts_;                            ///< 最大重试次数
    int current_retry_count_;                           ///< 当前重试次数

    /**
     * @brief 验证状态转换是否有效
     * @param from 起始状态
     * @param to 目标状态
     * @return 是否有效
     */
    bool IsValidTransition(ConnectionState from, ConnectionState to) const;
};

} // namespace network
} // namespace sqlcc
