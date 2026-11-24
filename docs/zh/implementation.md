# ops-rand API 实现状态

## 版本信息
- **当前版本**: v1.0.0
- **更新日期**: 2026-03-21
- **开发阶段**: Phase 1 - Generator & Uniform

---

## ✅ 已实现接口 (Phase 1)

### 版本管理
- `aclrandGetVersion` - 获取库版本号

### 生成器管理
- `aclrandCreateGenerator` - 创建随机数生成器
- `aclrandDestroyGenerator` - 销毁生成器

### 生成器配置
- `aclrandSetGeneratorSeed` - 设置生成器种子
- `aclrandSetGeneratorOffset` - 设置生成器偏移量
- `aclrandSetGeneratorStream` - 设置 ACL 流

### 随机数生成
- `aclrandGenerateUniform` - 生成均匀分布随机数 (FP32)

### 辅助工具
- `aclrandGetErrorString` - 获取错误字符串（待验证）

### 已实现算子
- **stateless_random_uniform_v2** (无状态均匀分布) - `src/stateless_random_uniform_v2/`

### 支持范围
- **生成器类型**: `ACLRAND_RNG_PSEUDO_DEFAULT`, `ACLRAND_RNG_PSEUDO_PHILOX4_32_10`
- **数据类型**: FP32 (`float`)
- **芯片**: Ascend 950

---

## ⏸️ 待实现接口

### Phase 2 - 分布扩展

**必须实现**：
- `aclrandGenerateUniformDouble` - 双精度均匀分布
- `aclrandGenerateUniformHalf` - 半精度均匀分布 (FP16)
- `aclrandGenerateNormal` - 单精度正态分布
- `aclrandGenerate` - 32 位原始随机数

**重要功能**：
- `aclrandGenerateNormalDouble` - 双精度正态分布
- `aclrandGenerateNormalHalf` - 半精度正态分布
- `aclrandGenerateLogNormal` - 对数正态分布
- `aclrandGenerateLogNormalDouble` - 双精度对数正态
- `aclrandGenerateLogNormalHalf` - 半精度对数正态
- `aclrandGenerateLongLong` - 64 位原始随机数

### Phase 3 - 引擎扩展

**必须实现**：
- `aclrandGeneratePoisson` - 泊松分布
- `aclrandCreateDiscreteDistribution` - 创建离散分布
- `aclrandDestroyDistribution` - 销毁离散分布
- `aclrandGenerateDiscrete` - 离散分布采样

**重要功能**：
- `aclrandSetQuasiRandomDimensions` - 设置准随机维度
- `aclrandGetDirectionVectors32` - 获取 Sobol 方向向量 (32位)
- `aclrandGetDirectionVectors64` - 获取 Sobol 方向向量 (64位)

**新增生成器类型**：
- `ACLRAND_RNG_PSEUDO_XORWOW`
- `ACLRAND_RNG_PSEUDO_MRG32K3A`
- `ACLRAND_RNG_PSEUDO_MTGP32`
- `ACLRAND_RNG_PSEUDO_LFSR113`
- `ACLRAND_RNG_PSEUDO_THREEFRY2_32_20`
- `ACLRAND_RNG_QUASI_SOBOL32`
- `ACLRAND_RNG_QUASI_SOBOL64`

### Phase 4 - 高级特性

**可选功能**：
- `aclrandCreateGeneratorHost` - Host 端生成器
- `ops_rand_state_philox4x32_10` - Device API 状态结构
- `ops_rand_init` - Device API 初始化
- `ops_rand_uniform` - Device API 均匀分布
- `ops_rand_uniform4` - Device API 批量生成
- `ops_rand_normal` - Device API 正态分布
- `aclrandSetOrdering` - 设置排序方式

---

## ⏸️ 待支持数据类型

### 必须实现
- FP16 (`aclFloat16`)
- FP64 (`double`)

### 重要功能
- BF16 (`bfloat16`)

### 可选功能
- INT32, UINT32
- INT64, UINT64

---

## 📊 实现进度统计

| 类别 | 已实现 | 待实现 | 总计 |
|------|--------|--------|------|
| API 接口 | 7 | 22 | 29 |
| 生成器类型 | 2 | 9 | 11 |
| 分布类型 | 1 | 7 | 8 |
| 数据类型 | 1 | 4 | 5 |
| 算子 | 1 | - | - |

**总体完成度**: Phase 1 (Generator & Uniform) ✅

---

## 📝 开发路线图

### Phase 1: Generator & Uniform ✅ (已完成)
- [x] 基础架构搭建
- [x] 生成器管理接口
- [x] Philox 生成器实现
- [x] Uniform 分布生成 (FP32)
- [x] stateless_random_uniform_v2 算子
- [x] 打包流程

### Phase 2: 分布扩展 (下一步)
- [ ] 多数据类型支持 (FP16/FP64)
- [ ] 正态分布
- [ ] 对数正态分布
- [ ] 原始随机数生成 (32/64位)
- [ ] Box-Muller 变换算法

### Phase 3: 引擎扩展
- [ ] 泊松分布
- [ ] 离散分布
- [ ] XORWOW 生成器
- [ ] Sobol 准随机生成器
- [ ] MRG32K3A 生成器

### Phase 4: 高级特性
- [ ] Host 端生成器
- [ ] Device API (AscendC 内部调用)
- [ ] 性能优化
- [ ] 多流并行支持

---

## 📌 当前限制

1. **数据类型**: 仅支持 FP32
2. **分布类型**: 仅支持 Uniform (均匀分布)
3. **生成器**: 仅支持 Philox4x32_10
4. **算子**: 仅实现 stateless_random_uniform_v2
5. **不支持**: 正态分布、泊松分布、离散分布、准随机序列

---

## 🔧 设计参考

- **API 设计**: 参考 rocrand/cuRAND
- **生成器算法**: Philox4x32_10 (计数器模式)
- **测试框架**: 自研轻量级测试框架

---

**文档维护**: 请在实现新接口时及时更新本文档