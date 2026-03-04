#!/bin/bash
# ----------------------------------------------------------------------------------------------------------
# Copyright (c) 2026 Huawei Technologies Co., Ltd.
# This program is free software, you can redistribute it and/or modify it under the terms and conditions of
# CANN Open Software License Agreement Version 2.0 (the "License").
# Please refer to the License for details. You may not use this file except in compliance with the License.
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
# INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
# See LICENSE in the root of the software repository for the full text of the License.
# ----------------------------------------------------------------------------------------------------------

set -e

BUILD_DIR=build
BUILD_OUT_DIR=build_out
BUILD_OP=""
RUN_TEST=OFF
ENABLE_PACKAGE=OFF
SOC_VERSION=""
THREAD_NUM=$(nproc)
BUILD_TYPE=Release
VERBOSE=""

# 获取脚本所在目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CANN_3RD_LIB_PATH="${SCRIPT_DIR}/third_party"

# ==========================
# 帮助信息
# ==========================
show_help() {
    cat << EOF
ops-rand Build Script

Usage: bash build.sh [OPTIONS]

Options:
  --op=NAME             Build specific operator (e.g., uniform)
  --run                 Run operator test after build
  --pkg                 Build package (RUN file)
  --soc=VERSION         Target SoC version (ascend950, ascend910b, etc.)
  -j[N]                 Parallel build threads (default: $(nproc))
  --build-type=TYPE     Build type: Release (default), Debug
  -v, --verbose         Verbose output
  --help                Show this help

Examples:
  # Basic build
  bash build.sh

  # Build for Ascend 950
  bash build.sh --soc=ascend950 -j16

  # Build with package
  bash build.sh --pkg --soc=ascend950

  # Build specific operator
  bash build.sh --op=uniform --run
EOF
    exit 0
}

# ==========================
# 解析参数
# ==========================
for arg in "$@"; do
    case $arg in
        --op=*)
            BUILD_OP="${arg#*=}"
            ;;
        --run)
            RUN_TEST=ON
            ;;
        --pkg)
            ENABLE_PACKAGE=ON
            ;;
        --soc=*)
            SOC_VERSION="${arg#*=}"
            ;;
        -j*)
            THREAD_NUM="${arg#-j}"
            ;;
        --build-type=*)
            BUILD_TYPE="${arg#*=}"
            ;;
        -v|--verbose)
            VERBOSE="-v"
            ;;
        --help|-h)
            show_help
            ;;
        *)
            echo "Unknown option: $arg"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# 验证必要参数
if [ -z "$SOC_VERSION" ] && [ "$ENABLE_PACKAGE" = "ON" ]; then
    echo "Error: --soc is required for packaging"
    echo "Example: bash build.sh --pkg --soc=ascend950"
    exit 1
fi

echo "=========================================="
echo "Build Configuration:"
echo "  BUILD_OP       = ${BUILD_OP:-all}"
echo "  RUN_TEST       = ${RUN_TEST}"
echo "  ENABLE_PACKAGE = ${ENABLE_PACKAGE}"
echo "  SOC_VERSION    = ${SOC_VERSION:-not set}"
echo "  BUILD_TYPE     = ${BUILD_TYPE}"
echo "  THREAD_NUM     = ${THREAD_NUM}"
echo "=========================================="

# ==========================
# 构建
# ==========================
rm -rf ${BUILD_DIR}
mkdir -p ${BUILD_DIR}

CMAKE_OPTIONS=""

# 基本选项
if [ -n "$SOC_VERSION" ]; then
    CMAKE_OPTIONS="${CMAKE_OPTIONS} -DSOC_VERSION=${SOC_VERSION}"
fi

CMAKE_OPTIONS="${CMAKE_OPTIONS} -DCMAKE_BUILD_TYPE=${BUILD_TYPE}"
CMAKE_OPTIONS="${CMAKE_OPTIONS} -DASCEND_CANN_PACKAGE_PATH=${_ASCEND_INSTALL_PATH}"
CMAKE_OPTIONS="${CMAKE_OPTIONS} -DASCEND_HOME_PATH=${ASCEND_HOME_PATH}"
CMAKE_OPTIONS="${CMAKE_OPTIONS} -DCANN_3RD_LIB_PATH=${CANN_3RD_LIB_PATH}"

# 测试选项
if [ -n "$BUILD_OP" ]; then
    CMAKE_OPTIONS="${CMAKE_OPTIONS} -DBUILD_TEST=ON -DTEST_NAME=${BUILD_OP}"
fi

# 打包选项
if [ "$ENABLE_PACKAGE" = "ON" ]; then
    CMAKE_OPTIONS="${CMAKE_OPTIONS} -DENABLE_PACKAGE=ON"
fi

echo "CMake Options: ${CMAKE_OPTIONS}"

# 执行 CMake 配置
cmake -B ${BUILD_DIR} ${CMAKE_OPTIONS}

# 执行构建
cmake --build ${BUILD_DIR} -j${THREAD_NUM} ${VERBOSE}

# 执行打包
if [ "$ENABLE_PACKAGE" = "ON" ]; then
    echo ""
    echo "=============== Building Package ==============="
    cmake --build ${BUILD_DIR} --target package ${VERBOSE}
    echo ""
    echo "Package created successfully!"
    echo ""
    # 显示生成的 .run 文件路径
    if ls ${BUILD_OUT_DIR}/*.run 1> /dev/null 2>&1; then
        echo "Generated package(s):"
        ls -la ${BUILD_OUT_DIR}/*.run
    else
        echo "Looking for package files..."
        find ${BUILD_OUT_DIR} -name "*.run" -type f 2>/dev/null || echo "No .run files found in ${BUILD_OUT_DIR}"
    fi
fi

# 安装
cmake --install ${BUILD_DIR}

echo ""
echo "=============== Build Completed ==============="
