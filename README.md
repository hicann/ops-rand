# ops-rand

## 🔥Latest News
- [2026/03] ops-rand仓对外开源

## 🚀概述

ops-rand是[CANN](https://hiascend.com/software/cann) （Compute Architecture for Neural Networks）算子库中提供的随机数生成库。

### 主要特性

- ✅ **模块化设计** - 支持动态添加算子模块，每个算子独立开发、编译和测试
- ✅ **标准 CMake 构建** - 跨平台编译支持，统一的构建流程
- ✅ **完整测试体系** - 基于自定义测试框架，支持自动化测试和超时控制
- ✅ **便捷打包** - 一键生成 .run 安装包，支持 install/uninstall/upgrade
- ✅ **版本管理** - 安装信息记录和版本追踪，支持升级管理
- ✅ **轻量高效** - 简洁的架构设计，避免过度工程化

## 📝 版本配套

当前仓库已验证通过的 CANN Toolkit 最低版本如下：

| CANN 版本 | 发布时间 | 分支 |
| --- | --- | --- |
| [CANN 9.0.0-beta.2](https://www.hiascend.com/developer/download/community/result?module=cann&cann=9.0.0-beta.2) | `2026/03/30` | master |

请根据实际 CPU 架构，从上述链接目录中自行选择对应的 `.run` 安装包，版本不得低于上表。

> **关于 Docker 镜像**：[环境部署](docs/zh/context/quick_install.md) 中的 Docker 示例使用内置 CANN 9.0.0 的镜像，高于上表的最低版本，二者不冲突。

toolkit 安装包文件名格式如下：

- `Ascend-cann-toolkit_${cann_version}_linux-aarch64.run`
- `Ascend-cann-toolkit_${cann_version}_linux-x86_64.run`

1. **安装社区版 CANN Toolkit**

    ```bash
    # 确保安装包具有可执行权限
    chmod +x Ascend-cann-toolkit_${cann_version}_linux-${arch}.run
    # 安装命令
    ./Ascend-cann-toolkit_${cann_version}_linux-${arch}.run --install --force --install-path=${install_path}
    ```
    - `${cann_version}`：表示 toolkit 安装包版本号，需满足上文的最低版本要求。
    - `${arch}`：表示 CPU 架构，如 `aarch64`、`x86_64`。
    - `${install_path}`：表示指定安装路径，默认安装在 `/usr/local/Ascend` 目录。

2. **配置环境变量**

   安装完成后，请先执行：

    ```bash
    source ${install_path}/cann/set_env.sh
    ```

   请将 `${install_path}` 替换为 toolkit 的实际安装目录，例如 `/usr/local/Ascend` 或 `${HOME}/Ascend`。

## ⚡️ 快速入门

### 编译与测试

详细的 build.sh 参数说明请参考 [build 参数说明](docs/zh/context/build.md)。

```bash
# 编译所有算子（默认 8 线程）
./build.sh

# 编译指定算子
./build.sh --ops=stateless_random_uniform_v2

# 编译并运行测试
./build.sh --run

# 编译并打包成 .run 文件（打包必须指定 --soc）
./build.sh --pkg --soc=ascend950

# 查看完整帮助信息
./build.sh --help
```

### 安装

安装包文件名格式为 `cann-950-ops-rand_${package_version}_linux-${arch}.run`，其中：

- `${package_version}`：ops-rand 软件包自身版本号（定义于仓库根目录 `version.cmake` 中 `set_package` 的 `VERSION` 字段，当前为 `9.0.0`）。**注意：该版本号与「版本配套」章节中已验证的 CANN Toolkit 版本（如 `9.0.0-beta.2`）相互独立，两者不一致属正常现象。**
- `${arch}`：CPU 架构，如 `aarch64`、`x86_64`。

操作类型参数 `--full`（全量安装）、`--upgrade`（升级）、`--uninstall`（卸载）三者必须且只能指定一个，缺省会报 `only support one type: full/upgrade/uninstall` 并退出。

```bash
# 标准安装（需要 root 权限）
sudo ./cann-950-ops-rand_${package_version}_linux-${arch}.run --full

# 查看安装包信息
./cann-950-ops-rand_${package_version}_linux-${arch}.run --help

# 安装到自定义路径
sudo ./cann-950-ops-rand_${package_version}_linux-${arch}.run --full --install-path=/opt/ascend

# 卸载
sudo ./cann-950-ops-rand_${package_version}_linux-${arch}.run --uninstall

# 升级
sudo ./cann-950-ops-rand_${package_version}_linux-${arch}.run --upgrade
```

## 📖 项目说明

### 支持的算子

当前支持的算子列表：

| 算子名称 | 描述 | 状态 |
|---------|------|------|
| [StatelessRandomUniformV2](src/stateless_random_uniform_v2/) | 生成无状态的均匀分布随机数，给定种子可复现结果 | ✅ 已实现 |

更多算子正在持续开发中...

### SoC 支持矩阵

| SoC 型号 | SOC_VERSION | 支持状态 |
|---------|-------------|---------|
| Ascend950 | ascend950dt_9595 | ✅ 默认支持 |
| Ascend910B | ascend910b3 | ❌ 暂不支持 |
| Ascend910_93 | ascend910_93 | ❌ 暂不支持 |
| Ascend910 | ascend910 | ❌ 暂不支持 |
| Ascend310P | ascend310p | ❌ 暂不支持 |
| Ascend310B | ascend310b | ❌ 暂不支持 |

## 🔍目录结构
关键目录如下，详细目录介绍参见[项目目录](./docs/zh/context/dir_structure.md)。
```
├── cmake                          # 项目工程编译目录
├── docs                           # 项目文档介绍
├── include                        # 头文件目录
├── lib                            # Generator API 实现代码
├── src                            # 源码目录
│   ├── stateless_random_uniform_v2 # 无状态均匀分布随机数算子
│   ├── common                     # 算子公共代码
│   ├── internal                   # 内部实现头文件
│   └── CMakeLists.txt             # 算子编译配置文件
├── scripts                        # 脚本目录，包含自定义算子、kernel构建相关配置文件
├── tests                          # 测试工程目录
├── CMakeLists.txt
├── README.md
├── build.sh                       # 项目工程编译脚本
├── install_deps.sh                # 安装依赖包脚本
└── requirements.txt               # 项目需要的第三方依赖包
```
## 📖学习教程

若您**已完成快速入门**学习，对本项目有了一定认知，并希望**深入了解和体验项目**，请访问如下文档。

这些文档提供了多样化的场景介绍和更全面的操作指导，方便您应用于各种AI业务场景。

1. [算子调用](docs/zh/invocation/quick_op_invocation.md)：编译部署并调用算子的指南，提供了多种编译算子包以及运行算子的方法（包括执行算子样例和UT）等。
2. [算子开发](docs/zh/develop/aicore_develop_guide.md)：基于本项目工程开发新算子的指南，提供了创建算子工程、实现Tiling和Kernel核心交付件等指导。
3. [算子调试调优](docs/zh/debug/op_debug_prof.md)：提供了常见的算子调试和调优方法，如DumpTensor、msProf、Simulator等。

## 💬相关信息

- [贡献指南](CONTRIBUTING.md)
- [安全声明](SECURITY.md)
- [许可证](LICENSE)
- [所属SIG](https://gitcode.com/cann/community/tree/master/CANN/sigs/ops-basic)

## 🤝联系我们

本项目功能和文档正在持续更新和完善中，欢迎您关注最新版本。

- **问题反馈**：通过GitCode[【Issues】](https://gitcode.com/cann/ops-rand/issues)提交问题
- **社区互动**：通过GitCode[【讨论】](https://gitcode.com/cann/ops-rand/discussions)参与交流
- **技术专栏**：通过GitCode[【Wiki】](https://gitcode.com/cann/ops-rand/wiki)获取技术文章
