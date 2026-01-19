# SQLCC 测试分级准入规范（正式版）

这是测试治理文档，不是写给"新人"的，是写给代码审查 / CI / 架构层面的。

## 一、测试等级定义

| Level | 含义 | 是否进 CI |
|-------|------|-----------|
| Unit | 单函数 / 类 | 必进 |
| Level1 | 基础库 / utils | 必进 |
| Level2 | 模块 / 子系统 | 必进（fast） |
| Level3 | 跨模块流程 | 选择性 |
| Level4+ | 系统 / 集成 | 非阻断 |

## 二、Level2 测试准入条件（必须全部满足）

### ✅ 可以进入 Level2

- 覆盖 **单一模块**或**强内聚子系统**
- 使用 **真实实现**（允许真实 IO）
- 可重复（不依赖执行顺序）
- 单次运行 ≤ 2 秒 / case
- 不依赖网络、时间、外部服务

### ❌ 不允许进入 Level2

| 类型 | 去向 |
|------|------|
| 性能 benchmark | level7 / benchmark |
| debug / fix 验证 | tests/debug |
| 跨子系统事务流 | Level3 |
| 不稳定 / 随机 | 禁止 |

## 三、Level2 强制分层（BUILD 层面）

| 分类 | tag | CI 默认 |
|------|-----|----------|
| fast | 无 | ✅ |
| slow | slow | ❌ |
| benchmark | benchmark | ❌ |
| debug | debug / manual | ❌ |

## 四、BUILD 编写规范（强制）

```bazel
cc_test(
    name = "xxx_fast",
    srcs = [...],
)

cc_test(
    name = "xxx_slow",
    tags = ["slow"],
)

test_suite(
    name = "all",
    tests = [":xxx_fast", ":xxx_slow"],
)
```

### ❌ 禁止：

- 一个 cpp 一个 target（除非必要）
- 未分类直接进 Level2
- debug 测试混入 all

## 五、CI 推荐矩阵

| 场景 | 命令 |
|------|------|
| PR | `--test_tag_filters=-slow,-benchmark,-debug` |
| Nightly | 全量 |
| Benchmark | `--test_tag_filters=benchmark` |

## 六、Reviewer Checklist（建议写进代码评审模板）

- [ ] 测试等级是否正确？
- [ ] 是否错误进入 Level2？
- [ ] 是否打了 slow / benchmark 标签？
- [ ] BUILD 是否可聚合？

## 最终评价（很重要）

你的工程 测试数量已经超过 90% 的开源数据库项目，

你现在需要的 不是更多测试，而是：

**测试治理、测试分层、CI 策略**

这一步你已经走对了。
