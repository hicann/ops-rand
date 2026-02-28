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
BUILD_OP=""
RUN_TEST=OFF

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
        *)
            echo "Unknown option: $arg"
            echo "Usage:"
            echo "  bash build.sh                 # 只编译库"
            echo "  bash build.sh --op=uniform      # 编译指定算子"
            echo "  bash build.sh --op=uniform --run # 编译并运行算子"
            exit 1
            ;;
    esac
done

echo "BUILD_OP=${BUILD_OP}, RUN_TEST=${RUN_TEST}"

# ==========================
# 构建
# ==========================
rm -rf ${BUILD_DIR}
mkdir -p ${BUILD_DIR}

CMAKE_OPTIONS="-DSOC_VERSION=${SOC_VERSION} -DASCEND_CANN_PACKAGE_PATH=${_ASCEND_INSTALL_PATH} -DASCEND_HOME_PATH="${ASCEND_HOME_PATH}""

if [ -n "$BUILD_OP" ]; then
    CMAKE_OPTIONS="${CMAKE_OPTIONS} -DBUILD_TEST=ON -DTEST_NAME=${BUILD_OP}"
fi

cmake -B ${BUILD_DIR} ${CMAKE_OPTIONS}
cmake --build ${BUILD_DIR} -j
cmake --install ${BUILD_DIR}
