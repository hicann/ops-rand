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

# normalize_soc_name 单元测试（对应 issue #7）
# 覆盖：小写/大写/混合大小写输入、中间含大写字母的型号（Ascend910B1）、未知型号退化行为

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
# shellcheck source=../build.sh
source "${SCRIPT_DIR}/build.sh"

PASS=0
FAIL=0

assert_normalize() {
    local input="$1" expected="$2"
    local actual
    actual=$(normalize_soc_name "${input}")
    if [ "${actual}" = "${expected}" ]; then
        PASS=$((PASS + 1))
        echo "[PASS] normalize_soc_name('${input}') = '${actual}'"
    else
        FAIL=$((FAIL + 1))
        echo "[FAIL] normalize_soc_name('${input}') = '${actual}', expected '${expected}'"
    fi
}

echo "========================================"
echo "  normalize_soc_name 单元测试"
echo "========================================"

echo "-- 当前已支持型号 (Ascend950) --"
assert_normalize "Ascend950" "Ascend950"
assert_normalize "ascend950" "Ascend950"
assert_normalize "ASCEND950" "Ascend950"
assert_normalize "aScEnD950" "Ascend950"

echo "-- 模拟未来扩展多段大小写型号 --"
SUPPORTED_SOCS=("Ascend950" "Ascend910B" "Ascend910B1" "Ascend910B2" "Ascend910B3" "Ascend910B4" "Ascend910_93" "Ascend310P")
assert_normalize "ascend910b" "Ascend910B"
assert_normalize "ASCEND910B" "Ascend910B"
assert_normalize "ascend910b1" "Ascend910B1"
assert_normalize "ASCEND910B1" "Ascend910B1"
assert_normalize "Ascend910B1" "Ascend910B1"
assert_normalize "ascend910_93" "Ascend910_93"
assert_normalize "ascend310p" "Ascend310P"

echo "-- 未知型号：退化为首字母大写、其余小写（由 validate_soc_version 拒绝）--"
assert_normalize "ascendabc" "Ascendabc"
assert_normalize "foo" "Foo"

echo "========================================"
echo "结果: ${PASS} passed, ${FAIL} failed"
echo "========================================"

[ "${FAIL}" -eq 0 ]
