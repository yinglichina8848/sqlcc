# SQLCC FDD 工作流规范

**版本**: v1.0  
**日期**: 2026-02-06  
**范围**: 功能驱动开发（Feature-Driven Development）

---

## 1. 目标

- 以功能分解为主线，形成“功能 → 计划 → 实现 → 验证”的闭环
- 与 SDD 规范协同，确保功能与规范一致
- 为迭代计划和任务拆解提供统一入口

---

## 2. FDD 五大核心活动（适配 SQLCC）

1. **建立整体模型**  
   - 输出：系统功能域全景图  
   - 文档：`FEATURE_DECOMPOSITION.md`

2. **建立功能列表**  
   - 输出：功能点清单  
   - 文档：`FEATURE_DECOMPOSITION.md`

3. **按功能计划**  
   - 输出：优先级矩阵、里程碑  
   - 文档：`FEATURE_PRIORITY_MATRIX.md`

4. **按功能设计**  
   - 输出：SDD 设计文档  
   - 文档：`docs/sdd/features/<feature>/design.md`

5. **按功能实现**  
   - 输出：任务清单 + 验证记录  
   - 文档：`docs/sdd/features/<feature>/tasks.md`  
   - 验证：`docs/sdd/features/<feature>/verification.md`

---

## 3. FDD 与 SDD 的协同关系

```
FDD 功能分解 → SDD 规范链路 → 任务拆解 → 验证闭环
```

**映射规则**：
- 每个 FDD 功能点必须对应一个 SDD feature
- 每个 SDD 任务必须回溯到 FDD 功能点

---

## 4. 输出物与验收

| 输出物 | 路径 | 验收标准 |
|---|---|---|
| 功能分解树 | `docs/fdd/FEATURE_DECOMPOSITION.md` | 功能覆盖核心模块 |
| 优先级矩阵 | `docs/fdd/FEATURE_PRIORITY_MATRIX.md` | 优先级与里程碑齐全 |
| SDD 四件套 | `docs/sdd/features/<feature>/` | requirements/design/tasks/verification |

---

## 5. 质量门禁

- 功能点必须有验收标准与验证方法  
- 所有功能点必须归档到版本规划或迭代计划  
- 通过文档审计后才能进入实现阶段

