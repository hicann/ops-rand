# 算子调用

## 前提条件

- 环境部署：调用项目算子之前，请先参考[环境部署](../context/quick_install.md)完成基础环境搭建。
- 调用算子列表：项目可调用的算子参见[算子列表](../op_list.md)。

## 编译执行

基于社区版CANN包对算子源码修改时，可采用如下方式进行源码编译：

- [ops-rand包](#ops-rand包)：选择整个项目编译生成的包称为ops-rand包，可**完整替换**CANN包对应部分。该包支持aclnn和图模式调用AI Core算子。

### ops-rand包

1. **编译ops-rand包**

    进入项目根目录，执行如下编译命令：

    > 说明：编译过程依赖第三方开源软件，联网场景会自动下载，离线编译场景需要自行安装。

    ```bash
    # 编译所有算子并生成安装包
    bash build.sh --pkg --soc=ascend950
    ```
    - --pkg：生成安装包（RUN文件）。
    - --soc：目标SoC版本，当前支持`ascend950`（Ascend 950PR/950DT产品）。

    若提示如下信息，说明编译成功。

    ```bash
    Self-extractable archive "cann-950-ops-rand_9.0.0_linux-x86_64.run" successfully created.
    Build package success: build_out/cann-950-ops-rand_9.0.0_linux-x86_64.run
    ```

    编译成功后，run包存放于项目根目录的build_out目录下。

2. **安装ops-rand包**

    ```bash
    # 安装命令
    ./build_out/cann-950-ops-rand_*_linux*.run
    ```

    ops-rand安装在`${ASCEND_HOME_PATH}/cann`路径中，`${ASCEND_HOME_PATH}`表示CANN软件安装目录。

3. **配置环境变量**

    ```bash
    source ${ASCEND_HOME_PATH}/cann/set_env.bash
    ```

4. **（可选）卸载ops-rand包**

    ```bash
    # 卸载命令
    ./${install_path}/cann/share/info/ops_rand/script/uninstall.sh
    ```

## 本地验证

通过项目根目录build.sh脚本，可快速调用算子和UT用例，验证项目功能是否正常，build参数介绍参见[build参数说明](../context/build.md)。

### 运行测试

```bash
# 编译并运行测试
bash build.sh --run

# 编译指定算子并运行测试
bash build.sh --op=stateless_random_uniform_v2 --run
```

执行测试后会打印执行结果，以stateless_random_uniform_v2算子为例，结果如下：

```
100% tests passed, 0 tests failed
```

### 编译选项说明

| 选项 | 说明 | 示例 |
| :--- | :--- | :--- |
| `--op=NAME` | 编译指定算子 | `--op=stateless_random_uniform_v2` |
| `--soc=VERSION` | 目标SoC版本 | `--soc=ascend950` |
| `--build-type=TYPE` | 构建类型（Release/Debug） | `--build-type=Debug` |
| `--run` | 编译后运行测试 | `--run` |
| `--pkg` | 生成安装包 | `--pkg` |
| `-j[N]` | 并行编译线程数 | `-j8` |
| `-v` | 详细输出 | `-v` |

### 常用命令示例

```bash
# 基本编译
bash build.sh

# 编译所有算子
bash build.sh --soc=ascend950

# 编译并运行测试
bash build.sh --run

# 编译指定算子并运行测试
bash build.sh --op=stateless_random_uniform_v2 --run

# 编译生成安装包
bash build.sh --pkg --soc=ascend950

# 多线程编译
bash build.sh -j16 --soc=ascend950

# 调试模式编译
bash build.sh --build-type=Debug --run

# 详细输出
bash build.sh -v --run
```

## 更多帮助

- [CANN 开发文档](https://www.hiascend.com/document)
- [Ascend C API 参考](https://hiascend.com/document/redirect/CannCommunityAscendCApi)
