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

    > 说明：编译过程依赖第三方开源软件，联网场景会自动下载，离线编译场景需要自行安装，具体参考[未联网编译](#未联网编译)。

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

### 未联网编译

若在没有连接互联网的环境下编译，需要提前准备好依赖的第三方软件，再进行源码编译。具体过程如下：

1. **检查基础环境是否完备**

    请确保已按[环境部署](../context/quick_install.md)完成基础环境搭建，包括CANN包安装、源码下载等。

    - 在联网环境中，进入[本项目主页](https://gitcode.com/cann/ops-rand)，通过`下载ZIP`或`clone`按钮，根据指导完成源码下载。
    - 连接离线环境，上传源码至您指定的目录下。若下载的是源码压缩包，请先进行解压。

2. **下载第三方软件依赖**

    在联网环境中提前下载第三方软件，目前有如下方式，请按需选择：

    - 方式1：根据[第三方软件依赖](#第三方软件依赖)提供的表格手动下载，若从其他地址下载，请确保版本号一致。

    - 方式2：通过[third_lib_download.py](../../../scripts/tools/third_lib_download.py)脚本一键下载，该脚本在本项目`scripts/tools/`目录，下载该脚本并执行如下命令：

        ```bash
        python ${scripts_dir}/third_lib_download.py
        ```

    ${scripts_dir}表示脚本存放路径，下载的第三方软件包默认存放在当前脚本所在目录。

3. **编译算子包**

    将下载好的第三方软件上传至离线环境，可存放在`third_party`目录或自定义目录下。**推荐前者，其编译命令与联网编译场景下的命令一致。**

    - **third\_party目录**（推荐）

        请在本项目根目录创建`third_party`目录（若有则无需创建），将第三方软件拷贝到该指定目录。此时编译命令与联网编译命令一致。

        ```bash
        bash build.sh --pkg --soc=ascend950
        ```

    - **自定义目录**

        在离线环境的任意位置新建`${cann_3rd_lib_path}`目录，将第三方软件拷贝到该目录，请确保该目录有权限访问。

        ```bash
        mkdir -p ${cann_3rd_lib_path}
        ```

        此时编译命令需在联网编译命令基础上额外增加`--cann_3rd_lib_path=${cann_3rd_lib_path}`用于指定第三方软件所在路径。假设存放路径为`/path/cann_3rd_lib_path`，命令如下：

        ```bash
        bash build.sh --pkg --soc=ascend950 --cann_3rd_lib_path=/path/cann_3rd_lib_path
        ```

4. **安装/卸载算子包**

    未联网和联网场景下编译得到的算子包结果一样，安装和卸载的操作命令也一样，具体参见下文步骤2和步骤4。

### 第三方软件依赖

本项目编译过程依赖的第三方开源软件列表如下：

| 开源软件 | 版本 | 下载地址 |
|---|---|---|
| makeself | 2.5.0 | [makeself-release-2.5.0-patch1.tar.gz](https://gitcode.com/cann-src-third-party/makeself/releases/download/release-2.5.0-patch1.0/makeself-release-2.5.0-patch1.tar.gz) |

若您的编译环境可以访问网络，编译脚本会自动联网下载第三方软件。否则，请参考[未联网编译](#未联网编译)手动下载第三方软件。

2. **安装ops-rand包**

    ```bash
    # 安装命令
    ./build_out/cann-950-ops-rand_*_linux*.run --full --install-path=${install_path}
    ```

    ops-rand安装在`${install_path}/cann`路径中，`${install_path}`表示CANN软件安装目录。

3. **配置环境变量**

    ```bash
    source ${install_path}/cann/set_env.bash
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
bash build.sh --ops=stateless_random_uniform_v2 --run
```

执行测试后会打印执行结果，以stateless_random_uniform_v2算子为例，结果如下：

```
100% tests passed, 0 tests failed
```

### 编译选项说明

| 选项 | 说明 | 示例 |
| :--- | :--- | :--- |
| `--ops=NAME` | 编译指定算子 | `--ops=stateless_random_uniform_v2` |
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
bash build.sh --ops=stateless_random_uniform_v2 --run

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
