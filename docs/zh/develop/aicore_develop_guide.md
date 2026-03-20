# AI Core算子开发指南

> 说明：
> 1. 算子开发过程中涉及的基本概念如Tiling、Kernel、Ascend C接口等，详细介绍请参考[《Ascend C算子开发》](https://hiascend.com/document/redirect/CannCommunityOpdevAscendC)。
> 2. AI Core算子是使用Ascend C语言开发，运行在AI Core硬件单元的算子。

本指南以ops-rand项目中的随机数算子为例，介绍新算子开发流程以及涉及的交付件。

1. [工程创建](#工程创建)：开发算子前，需完成环境部署并创建算子目录。
2. [算子定义](#算子定义)：算子功能说明与原型定义。
3. [Tiling实现](#Tiling实现)：实现Host侧Tiling策略。
4. [Kernel实现](#Kernel实现)：实现Device侧算子核函数。
5. [编译部署](#编译部署)：通过工程编译脚本完成自定义算子的编译和安装。
6. [算子验证](#算子验证)：通过常见算子调用方式，验证自定义算子功能。

## 工程创建

### 1. 环境部署

开发算子前，请先参考[环境部署](../context/quick_install.md)完成基础环境搭建。

### 2. 目录创建

在`src`目录下创建新的算子目录，目录名为算子名的小写下划线形式。

```bash
# 创建算子目录
mkdir -p src/${op_name}
mkdir -p src/${op_name}/arch35
mkdir -p src/${op_name}/tests
```

创建完成后，目录结构如下所示：

```
${op_name}/                              # 算子名的小写下划线形式
├── CMakeLists.txt                       # 算子编译配置文件
├── ${op_name}.cpp                       # Kernel入口文件
├── arch35/                              # Ascend950特有实现
│   └── ${op_name}.h                     # Kernel实现头文件
└── tests/                               # 测试用例目录
    ├── CMakeLists.txt
    └── test_${op_name}_example.cpp      # 算子测试用例
```

## 算子定义

开发算子前需要先确定目标算子的功能和计算逻辑。

以`stateless_random_uniform_v2`算子为例：

- **功能**：生成无状态的均匀分布随机数
- **输入**：seed（随机种子）、offset（偏移量）、shape（输出形状）
- **输出**：填充了随机数的张量
- **支持的数据类型**：FP32、FP16、BF16

### API接口定义

在`include/cann_ops_rand.h`中定义算子的对外API接口：

```cpp
#ifdef __cplusplus
extern "C" {
#endif

/* 无状态均匀分布随机数生成 */
ACLRAND_API aclError aclrandStatelessRandomUniformV2(
    uint64_t seed,
    uint64_t offset,
    int32_t alg,
    float *output,
    uint64_t n
);

#ifdef __cplusplus
}
#endif
```

## Tiling实现

### Tiling简介

因NPU中AI Core内部存储空间有限，无法一次性将整个张量数据加载到计算单元中处理，因此需要将输入张量切分为多个小块（Tile），逐块进行计算，这一过程称为Tiling。

### TilingData结构体定义

在Kernel实现头文件中定义TilingData结构体：

```cpp
// stateless_random_uniform_v2.h
struct StatelessRandomUniformV2TilingData {
    uint32_t blockNum;           // 核数
    uint32_t blockTilingSize;    // 每个核处理的数据量
    uint32_t tailBlockTilingSize; // 尾核处理的数据量
    uint32_t ubTilingSize;       // UB切分大小
    uint32_t alg;                // 算法类型
    uint32_t key[2];             // Philox算法密钥（来自seed）
    uint32_t counter[4];         // Philox算法计数器（来自offset）
};
```

### Tiling策略计算

在Host侧（Kernel入口函数或API调用前）计算Tiling参数：

```cpp
// 获取平台信息
auto ascendcPlatform = platform_ascendc::PlatformAscendCManager::GetInstance();
auto coreNum = ascendcPlatform->GetCoreNumAiv();  // 获取可用核数

// 计算Block切分参数
constexpr uint32_t CORE_ALIGN_SIZE = 512;  // 核对齐大小
constexpr uint32_t MIN_TILING_SIZE = 256;  // 最小切分大小
constexpr uint32_t BLOCK_SIZE_BYTES = 32;  // 块大小（字节）

auto outputDtypeSize = sizeof(float);
auto outputSize = n * outputDtypeSize;
auto coreAlignFactor = CORE_ALIGN_SIZE / outputDtypeSize;
auto blockFactor = CeilDiv(outputSize, coreNum);
auto blockAlignFactor = CeilDiv(blockFactor, coreAlignFactor) * coreAlignFactor;

tilingData->blockTilingSize = std::max(blockAlignFactor, MIN_TILING_SIZE);
tilingData->blockNum = CeilDiv(outputSize, tilingData->blockTilingSize);
tilingData->tailBlockTilingSize = outputSize - tilingData->blockTilingSize * (tilingData->blockNum - 1);

// 计算UB切分参数
uint64_t ubSize = 0;
ascendcPlatform->GetCoreMemSize(platform_ascendc::CoreMemType::UB, ubSize);
auto quarterUbSize = ubSize / 4;  // 使用1/4的UB空间
auto alignFactor = BLOCK_SIZE_BYTES / outputDtypeSize;  // 块对齐因子
tilingData->ubTilingSize = CeilDiv(quarterUbSize / outputDtypeSize, alignFactor) * alignFactor;

// 设置随机数种子参数
tilingData->key[0] = static_cast<uint32_t>(seed);
tilingData->key[1] = static_cast<uint32_t>(seed >> 32);
tilingData->counter[0] = static_cast<uint32_t>(offset);
tilingData->counter[1] = static_cast<uint32_t>(offset >> 32);
```

## Kernel实现

### Kernel代码结构

ops-rand项目的Kernel实现主要包含以下文件：

```
src/${op_name}/
├── ${op_name}.cpp           # Kernel入口文件 + API实现
└── arch35/
    └── ${op_name}.h         # Kernel类实现
```

### Kernel类定义

```cpp
// stateless_random_uniform_v2.h
template <typename T>
class StatelessRandomUniformV2 {
public:
    __aicore__ inline StatelessRandomUniformV2() {}
    __aicore__ inline void Init(
        GM_ADDR y,
        const StatelessRandomUniformV2TilingData* __restrict tilingData,
        TPipe* pipeIn
    );
    __aicore__ inline void Process();

private:
    __aicore__ inline void ParseTilingData(const StatelessRandomUniformV2TilingData* tilingData);
    __aicore__ inline void Skip(const uint64_t count);  // 跳过计数器
    __aicore__ inline void DataTypeHandle(LocalTensor<T>& yOutput, const uint32_t calCount);
    __aicore__ inline void CopyOut();

private:
    TPipe* pipe;

    // 队列和缓冲区
    TQue<QuePosition::VECOUT, BUFFER_NUM> outQueY_;      // 输出队列
    TBuf<QuePosition::VECCALC> philoxQueBuf_;            // Philox结果缓冲区
    GlobalTensor<T> outputGm_;                            // 全局输出张量

    // Tiling参数
    uint32_t blockNum_ = 0;
    uint32_t blockTilingSize_ = 0;
    uint32_t ubTilingSize_ = 0;

    // 随机数生成状态
    uint32_t key_[2] = {0};
    uint32_t counter_[4] = {0};
};
```

### Init函数实现

```cpp
template <typename T>
__aicore__ inline void StatelessRandomUniformV2<T>::Init(
    GM_ADDR y,
    const StatelessRandomUniformV2TilingData* tilingData,
    TPipe* pipeIn
) {
    // 1. 解析Tiling数据
    ParseTilingData(tilingData);

    // 2. 计算当前核的偏移量
    auto blockIdx = GetBlockIdx();
    blockOffSet_ = blockTilingSize_ * blockIdx;

    // 3. 处理尾核情况
    if (blockIdx == blockNum_ - 1) {
        currBlockTilingSize_ = tailBlockTilingSize_;
    } else {
        currBlockTilingSize_ = blockTilingSize_;
    }

    // 4. 初始化全局张量和队列
    outputGm_.SetGlobalBuffer((__gm__ T*)y);
    pipe = pipeIn;
    pipe->InitBuffer(outQueY_, BUFFER_NUM, ubTilingSize_ * sizeof(T));
    pipe->InitBuffer(philoxQueBuf_, ubTilingSize_ * sizeof(uint32_t));
}
```

### Process函数实现

```cpp
template <typename T>
__aicore__ inline void StatelessRandomUniformV2<T>::Process() {
    // 1. 计算并跳过偏移量
    auto groupCnt = (blockOffSet_ + RESULT_ELEMENT_CNT - 1) / RESULT_ELEMENT_CNT;
    Skip(groupCnt);

    // 2. 分UB循环处理
    ubLoopCnt_ = (currBlockTilingSize_ + ubTilingSize_ - 1) / ubTilingSize_;

    for (auto idx = 0; idx < ubLoopCnt_; idx++) {
        // 处理尾块
        currUbTilingSize_ = ubTilingSize_;
        if ((idx == ubLoopCnt_ - 1) && (currBlockTilingSize_ % ubTilingSize_ != 0)) {
            currUbTilingSize_ = currBlockTilingSize_ % ubTilingSize_;
        }

        currOffSet_ = blockOffSet_ + idx * ubTilingSize_;

        // 3. 生成随机数
        LocalTensor<uint32_t> philoxRes = philoxQueBuf_.Get<uint32_t>();
        LocalTensor<T> yOutput = outQueY_.AllocTensor<T>();

        // 使用Philox算法生成随机数
        PhiloxRandom<10>(
            philoxRes,
            {key_[0], key_[1]},
            {counter_[0], counter_[1], counter_[2], counter_[3]},
            currUbTilingSize_
        );

        // 4. 数据类型转换
        DataTypeHandle(yOutput, currUbTilingSize_);
        outQueY_.EnQue(yOutput);

        // 5. 拷贝输出
        CopyOut();

        // 6. 更新计数器
        groupCnt = (currUbTilingSize_ + RESULT_ELEMENT_CNT - 1) / RESULT_ELEMENT_CNT;
        Skip(groupCnt);
    }
}
```

### 数据类型转换

```cpp
template <typename T>
__aicore__ inline void StatelessRandomUniformV2<T>::DataTypeHandle(
    LocalTensor<T>& yOutput,
    const uint32_t calCount
) {
    if constexpr (AscendC::IsSameType<T, half>::value) {
        // FP16: 取10位尾数，指数设为15
        Uint16ToHalf(yOutput, calCount);
    } else if constexpr (AscendC::IsSameType<T, float>::value) {
        // FP32: 取23位尾数，指数设为127
        Uint32ToFloat(yOutput, calCount);
    } else if constexpr (AscendC::IsSameType<T, bfloat16_t>::value) {
        // BF16: 取7位尾数，指数设为127
        Uint16ToBfloat16(yOutput, calCount);
    }
}
```

### Kernel入口函数

```cpp
// stateless_random_uniform_v2.cpp
extern "C" __global__ __aicore__ void stateless_random_uniform_v2(
    GM_ADDR y,
    StatelessRandomUniformV2TilingData tilingData
) {
    TPipe pipe;
    StatelessRandomUniformV2<float> op;
    op.Init(y, &tilingData, &pipe);
    op.Process();
}
```

### Host侧API实现

```cpp
aclError aclrandStatelessRandomUniformV2(
    uint64_t seed,
    uint64_t offset,
    int32_t alg,
    float *output,
    uint64_t n
) {
    // 1. 参数校验
    if (output == nullptr || n == 0) {
        return ACL_ERROR;
    }

    // 2. 分配设备内存
    void *dev_out = nullptr;
    aclrtMalloc(&dev_out, n * sizeof(float), ACL_MEM_MALLOC_HUGE_FIRST);

    // 3. 计算Tiling参数
    auto ascendcPlatform = platform_ascendc::PlatformAscendCManager::GetInstance();
    StatelessRandomUniformV2TilingData *tilingData = new StatelessRandomUniformV2TilingData();
    // ... Tiling参数计算 ...

    // 4. 启动Kernel
    stateless_random_uniform_v2<<<tilingData->blockNum, nullptr, 0>>>(
        (__gm__ uint8_t *)dev_out, *tilingData
    );

    // 5. 拷贝结果到Host
    aclrtMemcpy(output, n * sizeof(float), dev_out, n * sizeof(float), ACL_MEMCPY_DEVICE_TO_HOST);

    // 6. 释放资源
    aclrtFree(dev_out);

    return ACL_SUCCESS;
}
```

## 编译部署

算子开发完成后，需对算子工程进行编译，生成ops-rand安装包。

### 1. 准备工作

参考[工程创建](#工程创建)完成基础环境搭建，同时检查算子开发交付件是否完备。

### 2. 配置环境变量

```bash
# 默认路径安装
source /usr/local/Ascend/cann/set_env.sh
# 指定路径安装
# source ${install_path}/cann/set_env.sh
```

### 3. 编译ops-rand包

进入项目根目录，执行编译命令：

```bash
# 编译所有算子
bash build.sh --pkg --soc=ascend950

# 编译指定算子
bash build.sh --pkg --soc=ascend950 --ops=${op_name}
```

若提示如下信息，说明编译成功：

```bash
Self-extractable archive "cann-950-ops-rand_9.0.0_linux-x86_64.run" successfully created.
```

### 4. 安装ops-rand包

```bash
./build_out/cann-950-ops-rand_*_linux*.run
```

ops-rand安装在`${ASCEND_HOME_PATH}/cann`路径中。

## 算子验证

```bash
# 编译并运行测试
bash build.sh --ops=${op_name} --run
```

## 随机数生成注意事项

### Philox算法

ops-rand使用Philox算法生成随机数，该算法具有以下特点：

1. **确定性**：相同的seed和offset产生相同的随机序列
2. **并行友好**：支持多核并行生成，通过counter偏移实现
3. **高质量**：通过多轮加密混淆保证随机性

### 数据类型转换公式

| 数据类型 | 尾数位数 | 指数值 | 转换公式 |
| :--- | :---: | :---: | :--- |
| FP32 | 23 | 127 | `(exp << 23) | mantissa` - 1.0f |
| FP16 | 10 | 15 | `(exp << 10) | mantissa` - 1.0h |
| BF16 | 7 | 127 | `(exp << 7) | mantissa` - 1.0bf |
