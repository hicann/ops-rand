/**
 * Copyright (c) 2026 Huawei Technologies Co., Ltd.
 * This program is free software, you can redistribute it and/or modify it under the terms and conditions of
 * CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */

/*!
 * \file stateless_random_uniform_v2.cpp
 * \brief
 */

#include "arch35/stateless_random_uniform_v2.h"

#include "acl/acl.h"
#include "platform/platform_info.h"
#include "tiling/platform/platform_ascendc.h"

#include "../common/utils.h"

using namespace StatelessRandom;

namespace {
constexpr uint32_t FLOAT_TILING_KEY = 101;
constexpr uint32_t FLOAT16_TILING_KEY = 102;
constexpr uint32_t BFLOAT16_TILING_KEY = 103;
constexpr uint32_t CORE_ALIGN_SIZE = 512;
constexpr uint32_t MIN_TILING_SIZE = 256;
constexpr uint32_t BLOCK_SIZE_BYTES = 32;
constexpr uint32_t UB_DIVISOR = 4;  // UB size divisor for tiling calculation

template <typename T1, typename T2>
inline T1 CeilDiv(const T1 a, const T2 b)
{
    if (b == 0) {
        return 0;
    }
    return (a + b - 1) / b;
}

/**
 * @brief Get data type size and tiling key based on aclDataType
 * @param dtype Input data type
 * @param dtypeSize Output: size of the data type in bytes
 * @param tilingKey Output: tiling key for the data type
 * @return ACL_SUCCESS on success, ACL_ERROR_INVALID_PARAM for unsupported dtype
 */
aclError GetDtypeInfo(aclDataType dtype, uint32_t& dtypeSize, uint32_t& tilingKey)
{
    switch (dtype) {
        case ACL_FLOAT:
            dtypeSize = sizeof(float);
            tilingKey = FLOAT_TILING_KEY;
            return ACL_SUCCESS;
        case ACL_FLOAT16:
            dtypeSize = sizeof(uint16_t);
            tilingKey = FLOAT16_TILING_KEY;
            return ACL_SUCCESS;
        case ACL_BF16:
            dtypeSize = sizeof(uint16_t);
            tilingKey = BFLOAT16_TILING_KEY;
            return ACL_SUCCESS;
        default:
            return ACL_ERROR_INVALID_PARAM;
    }
}

/**
 * @brief Calculate tiling parameters for the kernel
 * @param outputSize Total output size in bytes
 * @param dtypeSize Size of each element in bytes
 * @param platform AscendC platform instance
 * @param tilingData Output: calculated tiling parameters
 */
void CalcTilingData(
    size_t outputSize,
    uint32_t dtypeSize,
    platform_ascendc::PlatformAscendC* platform,
    StatelessRandomUniformV2TilingData& tilingData)
{
    auto coreNum = platform->GetCoreNumAiv();
    auto coreAlignFactor = CORE_ALIGN_SIZE / dtypeSize;
    auto blockFactor = CeilDiv(outputSize, coreNum);
    auto blockAlignFactor = CeilDiv(blockFactor, coreAlignFactor) * coreAlignFactor;

    tilingData.blockTilingSize = std::max(static_cast<uint32_t>(blockAlignFactor), MIN_TILING_SIZE);
    tilingData.blockNum = CeilDiv(outputSize, tilingData.blockTilingSize);
    tilingData.tailBlockTilingSize = outputSize - tilingData.blockTilingSize * (tilingData.blockNum - 1);

    uint64_t ubSize = 0;
    platform->GetCoreMemSize(platform_ascendc::CoreMemType::UB, ubSize);
    auto quarterUbSize = ubSize / UB_DIVISOR;
    auto alignFactor = BLOCK_SIZE_BYTES / dtypeSize;
    tilingData.ubTilingSize = CeilDiv(quarterUbSize / dtypeSize, alignFactor) * alignFactor;
}

/**
 * @brief Pack seed and offset into tiling data structure
 * @param seed Random seed value
 * @param offset Random offset value
 * @param alg Algorithm identifier
 * @param tilingKey Tiling key for data type
 * @param tilingData Output: tiling data with packed values
 */
void PackSeedAndOffset(
    uint64_t seed,
    uint64_t offset,
    int32_t alg,
    uint32_t tilingKey,
    StatelessRandomUniformV2TilingData& tilingData)
{
    tilingData.alg = alg;
    tilingData.tilingKey = tilingKey;
    tilingData.counter[0] = static_cast<uint32_t>(offset);
    tilingData.counter[1] = static_cast<uint32_t>(offset >> 32);
    tilingData.counter[2] = 0;
    tilingData.counter[3] = 0;
    tilingData.key[0] = static_cast<uint32_t>(seed);
    tilingData.key[1] = static_cast<uint32_t>(seed >> 32);
}
} // namespace

extern "C" __global__ __aicore__ void stateless_random_uniform_v2(
    GM_ADDR y, StatelessRandomUniformV2TilingData tilingData)
{
    KERNEL_TASK_TYPE_DEFAULT(KERNEL_TYPE_AIV_ONLY);
    TPipe pipe;
    if (tilingData.tilingKey == FLOAT_TILING_KEY) {
        StatelessRandomUniformV2<float> op;
        op.Init(y, &tilingData, &pipe);
        op.Process();
    } else if (tilingData.tilingKey == FLOAT16_TILING_KEY) {
        StatelessRandomUniformV2<half> op;
        op.Init(y, &tilingData, &pipe);
        op.Process();
    } else if (tilingData.tilingKey == BFLOAT16_TILING_KEY) {
        StatelessRandomUniformV2<bfloat16_t> op;
        op.Init(y, &tilingData, &pipe);
        op.Process();
    }
}

extern "C" aclError _aclrandStatelessRandomUniformV2(
    uint64_t seed, uint64_t offset, int32_t alg, void* output, uint64_t n, aclDataType dtype, aclrtStream stream)
{
    // 1. Parameter validation
    if (output == nullptr || n == 0) {
        return ACL_ERROR;
    }

    // 2. Get data type information
    uint32_t dtypeSize = 0;
    uint32_t tilingKey = 0;
    aclError ret = GetDtypeInfo(dtype, dtypeSize, tilingKey);
    if (ret != ACL_SUCCESS) {
        return ret;
    }

    // 3. Allocate device memory
    size_t outputSize = static_cast<size_t>(n) * dtypeSize;
    void* devOut = nullptr;
    if (aclrtMalloc(&devOut, outputSize, ACL_MEM_MALLOC_HUGE_FIRST) != ACL_SUCCESS) {
        return ACL_ERROR;
    }

    // 4. Calculate tiling parameters
    auto platform = platform_ascendc::PlatformAscendCManager::GetInstance();
    if (platform == nullptr) {
        aclrtFree(devOut);
        return ACL_ERROR;
    }
    StatelessRandomUniformV2TilingData tilingData;
    CalcTilingData(outputSize, dtypeSize, platform, tilingData);
    PackSeedAndOffset(seed, offset, alg, tilingKey, tilingData);

    // 5. Launch kernel
    stateless_random_uniform_v2<<<tilingData.blockNum, nullptr, stream>>>(
        reinterpret_cast<uint8_t*>(devOut), tilingData);

    // 6. Copy result back to host
    if (aclrtMemcpy(output, outputSize, devOut, outputSize, ACL_MEMCPY_DEVICE_TO_HOST) != ACL_SUCCESS) {
        aclrtFree(devOut);
        return ACL_ERROR;
    }

    // 7. Free device memory
    aclrtFree(devOut);

    return ACL_SUCCESS;
}
