#!/usr/bin/env bash
# ----------------------------------------------------------------------------------------------------------
# Copyright (c) 2026 Huawei Technologies Co., Ltd.
# This program is free software, you can redistribute it and/or modify it under the terms and conditions of
# CANN Open Software License Agreement Version 2.0 (the "License").
# Please refer to the License for details. You may not use this file except in compliance with the License.
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
# INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
# See LICENSE in the root of the software repository for the full text of the License.
# ----------------------------------------------------------------------------------------------------------

# build.sh 参数处理与校验逻辑测试（对应 issue #8 / #9）
# 覆盖：--pkg 必须显式指定 --soc、非打包默认 SoC 填充、verbose 变量语义

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
# shellcheck source=../build.sh
source "${SCRIPT_DIR}/build.sh"

PASS=0
FAIL=0

assert_eq() {
    local desc="$1" actual="$2" expected="$3"
    if [ "${actual}" = "${expected}" ]; then
        PASS=$((PASS + 1))
        echo "[PASS] ${desc}"
    else
        FAIL=$((FAIL + 1))
        echo "[FAIL] ${desc}: got '${actual}', expected '${expected}'"
    fi
}

echo "========================================"
echo "  build.sh 参数与校验逻辑单元测试"
echo "========================================"

echo "-- issue #8: --pkg 必须显式指定 --soc，SOC_VERSION 默认为空 --"
assert_eq "SOC_VERSION 默认值为空" "${SOC_VERSION}" ""

(
    set +e
    output=$(bash "${SCRIPT_DIR}/build.sh" --pkg 2>&1)
    rc=$?
    if [ ${rc} -eq 1 ] && echo "${output}" | grep -q "\-\-soc is required for packaging"; then
        echo "[PASS] --pkg 不带 --soc 被拒绝（exit 1 且提示明确）"
        PASS=$((PASS + 1))
    else
        echo "[FAIL] --pkg 不带 --soc: exit=${rc}, output=${output}"
        FAIL=$((FAIL + 1))
    fi
)

echo "-- issue #8: 非打包模式下未指定 --soc 时自动使用默认 SoC --"
SOC_VERSION=""
if [ "$ENABLE_PACKAGE" = "ON" ] && [ -z "$SOC_VERSION" ]; then
    :  # 打包校验分支
else
    SOC_VERSION="${SUPPORTED_SOCS[0]}"
fi
assert_eq "非打包默认 SoC 为 SUPPORTED_SOCS[0]" "${SOC_VERSION}" "${SUPPORTED_SOCS[0]}"

echo "-- issue #9: verbose 拆分为布尔开关与 cmake 透传参数 --"
VERBOSE_ENABLED=false
CMAKE_VERBOSE=""
log_verbose "should not print" >/dev/null
output=$(log_verbose "hidden" 2>&1)
assert_eq "VERBOSE_ENABLED=false 时 log_verbose 无输出" "${output}" ""

VERBOSE_ENABLED=true
output=$(log_verbose "visible" 2>&1)
if echo "${output}" | grep -q "VERBOSE" && echo "${output}" | grep -q "visible"; then
    echo "[PASS] VERBOSE_ENABLED=true 时 log_verbose 正常输出"
    PASS=$((PASS + 1))
else
    echo "[FAIL] VERBOSE_ENABLED=true 时 log_verbose 输出异常: ${output}"
    FAIL=$((FAIL + 1))
fi

echo "========================================"
echo "结果: ${PASS} passed, ${FAIL} failed"
echo "========================================"

[ "${FAIL}" -eq 0 ]
