# ops-rand API 参考

---

## 一、Phase 2：分布扩展 API

### 1.1 扩展数据类型 - Uniform 分布

#### aclrandGenerateUniformDouble

```cpp
/**
 * @brief 生成双精度均匀分布随机数
 *
 * 生成 (0.0, 1.0] 范围的双精度浮点数。
 *
 * @param generator 生成器句柄
 * @param outputData 输出缓冲区指针
 * @param n 生成数量
 * @return aclrandStatus_t
 *   - ACLRAND_STATUS_SUCCESS: 成功
 *   - ACLRAND_STATUS_NOT_CREATED: 生成器未创建
 *   - ACLRAND_STATUS_LAUNCH_FAILURE: Kernel 启动失败
 *
 * @note 与 rocrand_generate_uniform_double 对等
 */
aclrandStatus_t aclrandGenerateUniformDouble(
    aclrandGenerator_t generator,
    double *outputData,
    size_t n
);
```

#### aclrandGenerateUniformHalf

```cpp
/**
 * @brief 生成半精度均匀分布随机数
 *
 * 生成 (0.0, 1.0] 范围的半精度浮点数 (FP16)。
 *
 * @param generator 生成器句柄
 * @param outputData 输出缓冲区指针
 * @param n 生成数量
 * @return aclrandStatus_t
 *
 * @note 与 rocrand_generate_uniform_half 对等
 */
aclrandStatus_t aclrandGenerateUniformHalf(
    aclrandGenerator_t generator,
    aclFloat16 *outputData,
    size_t n
);
```

---

### 1.2 正态分布生成

#### aclrandGenerateNormal

```cpp
/**
 * @brief 生成单精度正态分布随机数
 *
 * 生成均值为 0.0，标准差为 1.0 的正态分布随机数。
 * 使用 Box-Muller 变换算法。
 *
 * @param generator 生成器句柄
 * @param outputData 输出缓冲区指针
 * @param n 生成数量（必须为偶数）
 * @return aclrandStatus_t
 *   - ACLRAND_STATUS_SUCCESS: 成功
 *   - ACLRAND_STATUS_NOT_CREATED: 生成器未创建
 *   - ACLRAND_STATUS_OUT_OF_RANGE: n 不是偶数
 *
 * @note Box-Muller 变换每次生成 2 个随机数，故 n 必须为偶数
 * @note 与 rocrand_generate_normal 对等
 */
aclrandStatus_t aclrandGenerateNormal(
    aclrandGenerator_t generator,
    float *outputData,
    size_t n
);
```

#### aclrandGenerateNormalDouble

```cpp
/**
 * @brief 生成双精度正态分布随机数
 *
 * @param generator 生成器句柄
 * @param outputData 输出缓冲区指针
 * @param n 生成数量（必须为偶数）
 * @return aclrandStatus_t
 */
aclrandStatus_t aclrandGenerateNormalDouble(
    aclrandGenerator_t generator,
    double *outputData,
    size_t n
);
```

#### aclrandGenerateNormalHalf

```cpp
/**
 * @brief 生成半精度正态分布随机数
 *
 * @param generator 生成器句柄
 * @param outputData 输出缓冲区指针
 * @param n 生成数量（必须为偶数）
 * @return aclrandStatus_t
 */
aclrandStatus_t aclrandGenerateNormalHalf(
    aclrandGenerator_t generator,
    aclFloat16 *outputData,
    size_t n
);
```

---

### 1.3 对数正态分布生成

#### aclrandGenerateLogNormal

```cpp
/**
 * @brief 生成单精度对数正态分布随机数
 *
 * 生成对数正态分布随机数，其中 ln(X) ~ N(mean, stddev)。
 *
 * @param generator 生成器句柄
 * @param outputData 输出缓冲区指针
 * @param n 生成数量
 * @param mean 对数均值
 * @param stddev 对数标准差
 * @return aclrandStatus_t
 *
 * @note 与 rocrand_generate_log_normal 对等
 */
aclrandStatus_t aclrandGenerateLogNormal(
    aclrandGenerator_t generator,
    float *outputData,
    size_t n,
    float mean,
    float stddev
);
```

#### aclrandGenerateLogNormalDouble

```cpp
aclrandStatus_t aclrandGenerateLogNormalDouble(
    aclrandGenerator_t generator,
    double *outputData,
    size_t n,
    double mean,
    double stddev
);
```

#### aclrandGenerateLogNormalHalf

```cpp
aclrandStatus_t aclrandGenerateLogNormalHalf(
    aclrandGenerator_t generator,
    aclFloat16 *outputData,
    size_t n,
    float mean,
    float stddev
);
```

---

### 1.4 原始随机数生成

#### aclrandGenerate

```cpp
/**
 * @brief 生成 32 位无符号整数随机数
 *
 * 生成原始的 32 位无符号整数随机数，均匀分布于 [0, UINT_MAX]。
 *
 * @param generator 生成器句柄
 * @param outputData 输出缓冲区指针
 * @param n 生成数量
 * @return aclrandStatus_t
 *
 * @note 与 rocrand_generate 对等
 */
aclrandStatus_t aclrandGenerate(
    aclrandGenerator_t generator,
    unsigned int *outputData,
    size_t n
);
```

#### aclrandGenerateLongLong

```cpp
/**
 * @brief 生成 64 位无符号整数随机数
 *
 * @param generator 生成器句柄
 * @param outputData 输出缓冲区指针
 * @param n 生成数量
 * @return aclrandStatus_t
 *
 * @note 与 rocrand_generate_long_long 对等
 */
aclrandStatus_t aclrandGenerateLongLong(
    aclrandGenerator_t generator,
    unsigned long long *outputData,
    size_t n
);
```

---

## 二、Phase 3：引擎扩展 API

### 2.1 泊松分布生成

#### aclrandGeneratePoisson

```cpp
/**
 * @brief 生成泊松分布随机数
 *
 * 生成参数为 lambda 的泊松分布随机数。
 *
 * @param generator 生成器句柄
 * @param outputData 输出缓冲区指针（unsigned int）
 * @param n 生成数量
 * @param lambda 泊松分布参数（必须 > 0）
 * @return aclrandStatus_t
 *   - ACLRAND_STATUS_SUCCESS: 成功
 *   - ACLRAND_STATUS_OUT_OF_RANGE: lambda <= 0
 *
 * @note 使用逆 CDF 查表法实现
 * @note 与 rocrand_generate_poisson 对等
 */
aclrandStatus_t aclrandGeneratePoisson(
    aclrandGenerator_t generator,
    unsigned int *outputData,
    size_t n,
    double lambda
);
```

---

### 2.2 离散分布

#### aclrandDiscreteDistribution_t

```cpp
/**
 * @brief 离散分布句柄类型
 */
typedef struct aclrandDiscreteDistribution_st *aclrandDiscreteDistribution_t;
```

#### aclrandCreateDiscreteDistribution

```cpp
/**
 * @brief 创建离散分布
 *
 * 根据给定的概率分布创建离散分布对象。
 * 使用 Alias Method 实现O(1)采样。
 *
 * @param discreteDistribution 输出离散分布句柄
 * @param probabilities 概率数组
 * @param size 分布大小
 * @return aclrandStatus_t
 *
 * @note 与 rocrand_create_discrete_distribution 对等
 */
aclrandStatus_t aclrandCreateDiscreteDistribution(
    aclrandDiscreteDistribution_t *discreteDistribution,
    const double *probabilities,
    size_t size
);
```

#### aclrandDestroyDistribution

```cpp
/**
 * @brief 销毁离散分布
 *
 * @param discreteDistribution 离散分布句柄
 * @return aclrandStatus_t
 */
aclrandStatus_t aclrandDestroyDistribution(
    aclrandDiscreteDistribution_t discreteDistribution
);
```

#### aclrandGenerateDiscrete

```cpp
/**
 * @brief 从离散分布生成随机数
 *
 * @param generator 生成器句柄
 * @param discreteDistribution 离散分布句柄
 * @param outputData 输出缓冲区指针
 * @param n 生成数量
 * @return aclrandStatus_t
 */
aclrandStatus_t aclrandGenerateDiscrete(
    aclrandGenerator_t generator,
    aclrandDiscreteDistribution_t discreteDistribution,
    unsigned int *outputData,
    size_t n
);
```

---

### 2.3 新增生成器类型枚举

```cpp
typedef enum {
    // 已实现 (Phase 1)
    ACLRAND_RNG_PSEUDO_DEFAULT = 0,
    ACLRAND_RNG_PSEUDO_PHILOX4_32_10 = 100,

    // Phase 3 新增
    ACLRAND_RNG_PSEUDO_XORWOW = 101,
    ACLRAND_RNG_PSEUDO_MRG32K3A = 102,
    ACLRAND_RNG_PSEUDO_MTGP32 = 103,
    ACLRAND_RNG_PSEUDO_LFSR113 = 104,
    ACLRAND_RNG_PSEUDO_THREEFRY2_32_20 = 105,

    // 准随机生成器
    ACLRAND_RNG_QUASI_SOBOL32 = 200,
    ACLRAND_RNG_QUASI_SOBOL64 = 201,
    ACLRAND_RNG_QUASI_SCRAMBLED_SOBOL32 = 202,
    ACLRAND_RNG_QUASI_SCRAMBLED_SOBOL64 = 203,

} aclrandRngType_t;
```

---

### 2.4 准随机生成器 API

#### aclrandSetQuasiRandomDimensions

```cpp
/**
 * @brief 设置准随机生成器的维度
 *
 * @param generator 生成器句柄
 * @param dimensions 维度数
 * @return aclrandStatus_t
 *
 * @note 仅对 Sobol 类型生成器有效
 * @note 与 rocrand_set_dimensions 对等
 */
aclrandStatus_t aclrandSetQuasiRandomDimensions(
    aclrandGenerator_t generator,
    unsigned int dimensions
);
```

#### aclrandGetDirectionVectors32

```cpp
/**
 * @brief 获取 32 位 Sobol 方向向量
 *
 * @param vectors 输出方向向量指针
 * @param numVectors 向量数量
 * @return aclrandStatus_t
 *
 * @note 与 rocrand_get_direction_vectors32 对等
 */
aclrandStatus_t aclrandGetDirectionVectors32(
    unsigned int **vectors,
    unsigned int numVectors
);
```

#### aclrandGetDirectionVectors64

```cpp
/**
 * @brief 获取 64 位 Sobol 方向向量
 */
aclrandStatus_t aclrandGetDirectionVectors64(
    unsigned long long **vectors,
    unsigned int numVectors
);
```

---

## 三、Phase 4：高级特性 API

### 3.1 Host 端生成器

#### aclrandCreateGeneratorHost

```cpp
/**
 * @brief 在 Host 端创建生成器
 *
 * 创建在 CPU 上执行的生成器，用于测试或调试。
 *
 * @param generator 输出生成器句柄
 * @param rngType 生成器类型
 * @return aclrandStatus_t
 *
 * @note 与 rocrand_create_generator_host 对等
 */
aclrandStatus_t aclrandCreateGeneratorHost(
    aclrandGenerator_t *generator,
    aclrandRngType_t rngType
);
```

---

### 3.2 Device API（设备端调用）

#### ops_rand_state_philox4x32_10

```cpp
/**
 * @brief Philox 生成器状态结构（设备端）
 *
 * 用于在 AscendC Kernel 内部直接调用随机数生成。
 */
struct ops_rand_state_philox4x32_10 {
    unsigned int counter[4];    // 计数器
    unsigned int key[2];        // 密钥
    unsigned int result[4];     // 缓存结果
    int offset;                 // 当前偏移
};

typedef struct ops_rand_state_philox4x32_10 ops_rand_state_philox4x32_10;
```

#### ops_rand_init

```cpp
/**
 * @brief 初始化 Philox 状态
 *
 * @param state 状态指针
 * @param seed 种子
 * @param offset 偏移
 */
__device__ void ops_rand_init(
    ops_rand_state_philox4x32_10 *state,
    unsigned long long seed,
    unsigned long long offset
);
```

#### ops_rand_uniform

```cpp
/**
 * @brief 设备端生成单个均匀分布随机数
 *
 * @param state 生成器状态
 * @return float (0.0, 1.0] 范围的随机数
 */
__device__ float ops_rand_uniform(ops_rand_state_philox4x32_10 *state);
```

#### ops_rand_uniform4

```cpp
/**
 * @brief 设备端批量生成 4 个均匀分布随机数
 *
 * @param state 生成器状态
 * @param output 输出数组（4 个元素）
 */
__device__ void ops_rand_uniform4(
    ops_rand_state_philox4x32_10 *state,
    float output[4]
);
```

#### ops_rand_normal

```cpp
/**
 * @brief 设备端生成单个正态分布随机数
 *
 * 使用 Box-Muller 变换。
 *
 * @param state 生成器状态
 * @return float ~N(0, 1) 的随机数
 */
__device__ float ops_rand_normal(ops_rand_state_philox4x32_10 *state);
```

---

### 3.3 辅助函数

#### aclrandSetOrdering

```cpp
/**
 * @brief 设置结果排序方式
 *
 * @param generator 生成器句柄
 * @param ordering 排序方式
 * @return aclrandStatus_t
 *
 * @note 与 rocrand_set_ordering 对等
 */
typedef enum {
    ACLRAND_ORDERING_PSEUDO_BEST = 100,
    ACLRAND_ORDERING_PSEUDO_SEEDED = 101,
    ACLRAND_ORDERING_PSEUDO_LEGACY = 102,
    ACLRAND_ORDERING_QUASI_DEFAULT = 200,
} aclrandOrdering_t;

aclrandStatus_t aclrandSetOrdering(
    aclrandGenerator_t generator,
    aclrandOrdering_t ordering
);
```

---

## 四、扩展错误码定义

```cpp
typedef enum {
    // Phase 1 已定义
    ACLRAND_STATUS_SUCCESS = 0,
    ACLRAND_STATUS_VERSION_MISMATCH = 100,
    ACLRAND_STATUS_NOT_CREATED = 101,
    ACLRAND_STATUS_ALLOCATION_FAILED = 102,
    ACLRAND_STATUS_TYPE_ERROR = 103,
    ACLRAND_STATUS_OUT_OF_RANGE = 104,

    // Phase 2+ 新增
    ACLRAND_STATUS_LAUNCH_FAILURE = 105,    // Kernel 启动失败
    ACLRAND_STATUS_NOT_INITIALIZED = 106,   // 未初始化
    ACLRAND_STATUS_ARCH_MISMATCH = 107,     // 架构不匹配
    ACLRAND_STATUS_INTERNAL_ERROR = 108,    // 内部错误
    ACLRAND_STATUS_NOT_IMPLEMENTED = 109,   // 功能未实现

} aclrandStatus_t;
```

---

## 五、API 实现优先级

### 5.1 P0（必须实现）

| API | Phase | 说明 |
|-----|-------|------|
| `aclrandGenerateUniformDouble` | 2 | FP64 支持 |
| `aclrandGenerateUniformHalf` | 2 | FP16 支持 |
| `aclrandGenerateNormal` | 2 | 正态分布 |
| `aclrandGenerate` | 2 | 原始随机数 |

### 5.2 P1（高优先级）

| API | Phase | 说明 |
|-----|-------|------|
| `aclrandGenerateNormalDouble` | 2 | 双精度正态 |
| `aclrandGenerateLogNormal` | 2 | 对数正态 |
| `aclrandGenerateLongLong` | 2 | 64 位随机数 |

### 5.3 P2（中优先级）

| API | Phase | 说明 |
|-----|-------|------|
| `aclrandGeneratePoisson` | 3 | 泊松分布 |
| XORWOW 生成器 | 3 | 快速生成器 |
| Sobol 准随机 | 3 | 低差异序列 |

### 5.4 P3（低优先级）

| API | Phase | 说明 |
|-----|-------|------|
| Device API | 4 | 设备端调用 |
| MRG32k3a 生成器 | 3 | 高质量生成器 |
| ThreeFry 生成器 | 3 | 密码学强度 |