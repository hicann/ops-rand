# 项目目录

> 本章罗列的部分目录是可选的，请以实际交付件为准。尤其**单算子目录**，不同场景下交付件有差异。

项目全量目录层级介绍如下：
```
├── cmake                                               # 项目工程编译目录
│   └── ...
├── common                                              # 项目公共头文件和公共代码
│   ├── CMakeLists.txt
│   └── inc                                             # 公共头文件目录
├── docs                                                # 项目相关文档目录
│   ├── README.md                                       # 文档目录索引
│   └── zh                                              # 中文文档目录
│       ├── op_list.md                                  # 算子列表
│       ├── context                                     # 公共文档目录
│       ├── invocation                                  # 算子调用文档目录
│       ├── develop                                     # 算子开发文档目录
│       └── debug                                       # 调试调优文档目录
├── include                                             # 头文件目录
│   └── cann_ops_rand.h                                 # ops-rand API 头文件
├── lib                                                 # Generator API 实现目录
│   ├── generator.cpp                                   # Generator API 实现
│   ├── generator.h                                     # Generator 内部头文件
│   └── tests                                           # API 测试用例目录
├── scripts                                             # 脚本目录，包含自定义算子、Kernel构建相关配置文件
│   └── ...
├── src                                                 # 源码目录
│   ├── CMakeLists.txt                                  # 算子编译入口
│   ├── common                                          # 算子公共代码
│   │   └── ...
│   ├── internal                                        # 内部实现头文件
│   │   └── ops_impl.h                                  # 算子声明头文件
│   └── stateless_random_uniform_v2                    # 无状态均匀分布随机数算子
│       └── ...
├── tests                                               # 项目级测试目录（可选）
├── third_party                                         # 第三方依赖目录
│   └── ...
├── CMakeLists.txt                                      # 项目工程cmakelist入口
├── CONTRIBUTING.md                                     # 项目贡献指南文件
├── LICENSE                                             # 项目开源许可证信息
├── OAT.xml                                             # 配置脚本，代码仓工具使用，用于检查License是否规范
├── README.md                                           # 项目工程总介绍文档
├── QUICKSTART.md                                       # 快速入门指南
├── SECURITY.md                                         # 项目安全声明文件
├── build.sh                                            # 项目工程编译脚本
├── classify_rule.yaml                                  # 组件划分信息
├── install_deps.sh                                     # 项目安装依赖包脚本
├── requirements.txt                                    # 项目的第三方依赖包
└── version.cmake                                       # 项目版本信息
```

## 目录说明

### 核心目录

| 目录/文件 | 说明 |
| :--- | :--- |
| `src/` | 算子源码目录，包含所有算子的实现代码 |
| `src/stateless_random_uniform_v2/` | 无状态均匀分布随机数算子目录 |
| `src/common/` | 算子公共代码，包含通用工具函数和数据结构 |
| `src/internal/` | 内部实现头文件，封装算子声明 |
| `lib/` | Generator API 实现目录 |
| `include/` | API头文件目录 |
| `cmake/` | CMake编译配置文件 |

### 文档目录

| 目录/文件 | 说明 |
| :--- | :--- |
| `docs/` | 项目文档目录 |
| `docs/zh/` | 中文文档目录 |
| `docs/zh/context/` | 公共文档，如环境部署、目录介绍等 |
| `docs/zh/invocation/` | 算子调用相关文档 |
| `docs/zh/develop/` | 算子开发相关文档 |
| `docs/zh/debug/` | 调试调优相关文档 |

### 构建相关

| 文件 | 说明 |
| :--- | :--- |
| `build.sh` | 项目编译脚本，支持多种编译选项 |
| `CMakeLists.txt` | CMake配置文件 |
| `install_deps.sh` | 依赖安装脚本 |
| `requirements.txt` | Python依赖包列表 |

## 算子目录结构

每个算子目录（如`src/stateless_random_uniform_v2/`）的典型结构如下：

```
${op_name}/                              # 算子名的小写下划线形式
├── CMakeLists.txt                       # 算子编译配置文件
├── ${op_name}.cpp                       # Kernel入口文件，包含主函数和调度逻辑
├── arch35/                              # Ascend950特有实现
│   └── ${op_name}.h                     # Kernel实现头文件
└── tests/                               # 测试用例目录
    ├── CMakeLists.txt
    └── test_${op_name}_example.cpp      # 算子测试用例
```

> **说明**：不同算子的交付件可能有差异，请以实际目录为准。
