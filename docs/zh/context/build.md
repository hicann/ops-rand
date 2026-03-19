# build.sh 参数说明

本文档介绍ops-rand项目中`build.sh`脚本的功能和参数说明。

## 功能概述

`build.sh`是ops-rand项目的编译脚本，提供以下功能：

- 编译算子源码
- 运行测试用例
- 生成安装包（RUN文件）
- 支持指定算子编译
- 支持多线程编译

## 使用方法

```bash
bash build.sh [OPTIONS]
```

## 参数说明

| 参数 | 说明 | 默认值 |
| :--- | :--- | :--- |
| `--op=NAME` | 编译指定算子，多个算子用逗号分隔 | all（编译所有算子） |
| `--soc=VERSION` | 目标SoC版本 | ascend950 |
| `--build-type=TYPE` | 构建类型：Release（发布版）、Debug（调试版） | Release |
| `--run` | 编译后运行测试用例 | OFF |
| `--pkg` | 生成安装包（RUN文件） | OFF |
| `-j[N]` | 并行编译线程数 | nproc（CPU核心数） |
| `-v, --verbose` | 显示详细编译输出 | 关闭 |
| `-h, --help` | 显示帮助信息 | - |

## 支持的SoC版本

| SoC版本 | 对应产品 |
| :--- | :--- |
| ascend950 | Ascend 950PR / Ascend 950DT |

## 命令示例

### 基本编译

```bash
# 编译所有算子
bash build.sh

# 编译并运行测试
bash build.sh --run
```

### 指定算子编译

```bash
# 编译指定算子
bash build.sh --op=stateless_random_uniform_v2

# 编译多个算子（示例）
bash build.sh --op=stateless_random_uniform_v2,other_op

# 编译指定算子并运行测试
bash build.sh --op=stateless_random_uniform_v2 --run
```

### 生成安装包

```bash
# 生成ops-rand安装包
bash build.sh --pkg --soc=ascend950

# 多线程编译并生成安装包
bash build.sh --pkg --soc=ascend950 -j16
```

### 调试模式

```bash
# Debug模式编译
bash build.sh --build-type=Debug

# 详细输出
bash build.sh -v --run
```

### 组合使用

```bash
# 编译指定算子、Debug模式、运行测试、详细输出
bash build.sh --op=stateless_random_uniform_v2 --build-type=Debug --run -v

# 多线程编译安装包
bash build.sh --pkg --soc=ascend950 -j16 -v
```

## 编译输出

编译成功后，输出目录结构如下：

```
build/                           # 编译中间文件目录
build_out/                       # 编译输出目录
└── cann-950-ops-rand_*.run      # 安装包（使用--pkg时生成）
```

## 常见问题

### 1. 编译失败：找不到bisheng编译器

**原因**：bisheng编译器未安装或未配置到PATH环境变量。

**解决方案**：
```bash
# 检查bisheng是否安装
bisheng --version

# 如未安装，请参考环境部署文档安装CANN toolkit包
```

### 2. 编译失败：找不到ASCEND_HOME_PATH

**原因**：未配置CANN环境变量。

**解决方案**：
```bash
# 配置环境变量
source /usr/local/Ascend/cann/set_env.sh
```

### 3. 测试运行失败

**原因**：可能缺少运行依赖或NPU设备未正确配置。

**解决方案**：
```bash
# 检查NPU设备
npu-smi info

# 检查CANN环境
cat /usr/local/Ascend/ascend-toolkit/latest/opp/version.info
```
