# 算子列表

> 说明：
> - **算子目录**：目录名为算子名小写下划线形式，每个目录承载该算子所有交付件，包括代码实现、文档等，目录介绍参见[项目目录](./context/dir_structure.md)。
> - **算子执行硬件单元**：项目中提到的算子指AI Core算子。

项目提供的所有算子分类和算子列表如下：

| 算子分类 | 算子目录 | op_kernel | op_host | op_api | op_graph | 算子执行硬件单元 | 说明 |
| :--- | :--- | :---: | :---: | :---: | :---: | :--- | :--- |
| rand | [stateless_random_uniform_v2](#stateless_random_uniform_v2) | √ | - | - | - | AI Core | 生成无状态的均匀分布随机数，给定种子可复现结果。 |

## 算子详细说明

### stateless_random_uniform_v2

生成无状态的均匀分布随机数，通过指定种子(seed)确保结果可复现。

**目录位置**：`src/stateless_random_uniform_v2/`

**支持的芯片**：Ascend 950

**支持的数据类型**：FP32、FP16、BF16

**参数**：
- seed：随机种子（uint64_t）
- offset：偏移量（uint64_t）
- alg：算法类型（int32_t，当前仅支持0表示Philox算法）
- output：输出缓冲区指针（void*）
- n：生成数量（uint64_t）
- dtype：数据类型（int32_t，0=FP32, 1=FP16, 2=BF16）

**输出**：
- output：填充了随机数的张量，值范围固定为 [0, 1)

**特点**：
- 相同的种子和偏移量会产生相同的随机数序列
- 适用于需要可复现结果的场景
- 输出范围固定为 [0, 1)，不支持自定义范围

## 算子分类说明

### rand 类算子

ops-rand是[CANN](https://hiascend.com/software/cann)（Compute Architecture for Neural Networks）算子库中提供的**随机数生成库**，主要提供以下功能：

- **stateless_random_uniform_v2**：无状态版本的均匀分布随机数生成器，通过指定种子(seed)确保结果可复现。

## 支持的芯片版本

| 芯片类型 | 支持状态 |
| :--- | :---: |
| Ascend 950PR | √ |
| Ascend 950DT | √ |

> **说明**：本项目当前主要支持 Ascend 950 系列芯片，其他芯片版本的支持情况请关注后续更新。

## 快速开始

如需快速体验算子调用，请参考：
- [环境部署](./context/quick_install.md)：搭建基础环境
- [算子调用](./invocation/quick_op_invocation.md)：编译部署并调用算子
