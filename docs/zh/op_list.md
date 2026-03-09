# 算子列表

> 说明：
> - **算子目录**：目录名为算子名小写下划线形式，每个目录承载该算子所有交付件，包括代码实现、文档等，目录介绍参见[项目目录](./context/dir_structure.md)。
> - **算子执行硬件单元**：项目中提到的算子指AI Core算子。

项目提供的所有算子分类和算子列表如下：

| 算子分类 | 算子目录 | op_kernel | op_host | op_api | op_graph | 算子执行硬件单元 | 说明 |
| :--- | :--- | :---: | :---: | :---: | :---: | :--- | :--- |
| rand | [uniform](#uniform) | √ | - | - | - | AI Core | 生成均匀分布的随机数，支持FP32、FP16数据类型。 |
| rand | [stateless_random_uniform_v2](#stateless_random_uniform_v2) | √ | - | - | - | AI Core | 生成无状态的均匀分布随机数，给定种子可复现结果。 |

## 算子详细说明

### uniform

生成服从均匀分布的随机数，输出值在指定范围内均匀分布。

**目录位置**：`src/uniform/`

**支持的芯片**：Ascend 950

**支持的数据类型**：FP32、FP16

**输入**：
- shape：输出张量的形状
- dtype：输出张量的数据类型

**输出**：
- output：填充了随机数的张量，值范围 [from, to)

### stateless_random_uniform_v2

生成无状态的均匀分布随机数，通过指定种子(seed)确保结果可复现。

**目录位置**：`src/stateless_random_uniform_v2/`

**支持的芯片**：Ascend 950

**支持的数据类型**：FP32、FP16、BF16

**输入**：
- shape：输出张量的形状
- seed：随机种子
- offset：偏移量
- from：范围下界（默认0.0）
- to：范围上界（默认1.0）

**输出**：
- output：填充了随机数的张量，值范围 [from, to)

**特点**：
- 相同的种子和偏移量会产生相同的随机数序列
- 适用于需要可复现结果的场景

## 算子分类说明

### rand 类算子

ops-rand是[CANN](https://hiascend.com/software/cann)（Compute Architecture for Neural Networks）算子库中提供的**随机数生成库**，主要提供以下功能：

- **uniform**：生成服从均匀分布的随机数，范围可指定为[0, 1)或自定义区间。
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
