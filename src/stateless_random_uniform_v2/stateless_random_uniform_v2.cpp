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

#include <algorithm>

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
constexpr uint32_t UINT32_BITS = 32;
constexpr uint32_t UB_DIVISOR = 4; // UB size divisor for tiling calculation
// Upper bound of output elements: prevents excessive device memory allocation (DoS)
// and keeps every offset/product in the tiling chain safely within 64-bit range
constexpr uint64_t MAX_OUTPUT_ELEMENTS = 1024ULL * 1024ULL * 1024ULL; // 1G elements

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
 * @brief Resolve dtype information and validate the requested output size
 * @param n Number of output elements
 * @param dtype Input data type
 * @param dtypeSize Output: size of the data type in bytes
 * @param tilingKey Output: tiling key for the data type
 * @return ACL_SUCCESS on success, ACL_ERROR_INVALID_PARAM for unsupported dtype
 *         or output size over the per-call limit
 */
aclError GetOutputSizeInfo(uint64_t n, aclDataType dtype, uint32_t& dtypeSize, uint32_t& tilingKey)
{
    aclError ret = GetDtypeInfo(dtype, dtypeSize, tilingKey);
    if (ret != ACL_SUCCESS) {
        return ret;
    }
    // GetDtypeInfo returns non-zero dtypeSize for every supported dtype; keep the
    // explicit check so the division below can never see a zero divisor
    if (dtypeSize == 0) {
        return ACL_ERROR_INVALID_PARAM;
    }
    // Upper bound prevents excessive device memory allocation (DoS); the overflow
    // check keeps n * dtypeSize from wrapping around
    if (n > MAX_OUTPUT_ELEMENTS || n > SIZE_MAX / dtypeSize) {
        return ACL_ERROR_INVALID_PARAM;
    }
    return ACL_SUCCESS;
}

/**
 * @brief Calculate tiling parameters for the kernel
 * @param numOfOutput Total number of output elements
 * @param dtypeSize Size of each element in bytes
 * @param platform AscendC platform instance
 * @param tilingData Output: calculated tiling parameters
 * @return ACL_SUCCESS on success, ACL_ERROR_INVALID_PARAM if the output size
 *         exceeds the addressing range of uint32 tiling fields
 */
aclError CalcTilingData(
    uint64_t numOfOutput, uint32_t dtypeSize, platform_ascendc::PlatformAscendC* platform,
    StatelessRandomUniformV2TilingData& tilingData)
{
    auto coreNum = platform->GetCoreNumAiv();
    // All block-level tiling fields below are in ELEMENT count (not bytes),
    // consistent with how the kernel consumes them
    auto coreAlignFactor = CORE_ALIGN_SIZE / dtypeSize;
    auto blockFactor = CeilDiv(numOfOutput, coreNum);
    auto blockAlignFactor = CeilDiv(blockFactor, coreAlignFactor) * coreAlignFactor;

    // Guard against truncation: elements handled by a single block must fit uint32_t
    if (blockAlignFactor > UINT32_MAX) {
        return ACL_ERROR_INVALID_PARAM;
    }
    tilingData.blockTilingSize = std::max(static_cast<uint32_t>(blockAlignFactor), MIN_TILING_SIZE);
    tilingData.blockNum = CeilDiv(numOfOutput, tilingData.blockTilingSize);
    // 64-bit intermediate to avoid uint32_t overflow of blockTilingSize * (blockNum - 1)
    uint64_t processedSize = static_cast<uint64_t>(tilingData.blockTilingSize) * (tilingData.blockNum - 1);
    tilingData.tailBlockTilingSize = static_cast<uint32_t>(numOfOutput - processedSize);

    uint64_t ubSize = 0;
    platform->GetCoreMemSize(platform_ascendc::CoreMemType::UB, ubSize);
    auto quarterUbSize = ubSize / UB_DIVISOR;
    auto alignFactor = BLOCK_SIZE_BYTES / dtypeSize;
    tilingData.ubTilingSize = CeilDiv(quarterUbSize / dtypeSize, alignFactor) * alignFactor;
    return ACL_SUCCESS;
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
    uint64_t seed, uint64_t offset, int32_t alg, uint32_t tilingKey, StatelessRandomUniformV2TilingData& tilingData)
{
    tilingData.alg = alg;
    tilingData.tilingKey = tilingKey;
    // Philox算法使用128位counter(4×32位)，offset仅64位，高64位填0
    tilingData.counter[0] = static_cast<uint32_t>(offset);
    tilingData.counter[1] = static_cast<uint32_t>(offset >> UINT32_BITS);
    tilingData.counter[2] = 0; // 2：数组下标
    tilingData.counter[3] = 0; // 3：数组下标
    tilingData.key[0] = static_cast<uint32_t>(seed);
    tilingData.key[1] = static_cast<uint32_t>(seed >> UINT32_BITS);
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

    // 2. Validate dtype and output size
    uint32_t dtypeSize = 0;
    uint32_t tilingKey = 0;
    aclError ret = GetOutputSizeInfo(n, dtype, dtypeSize, tilingKey);
    if (ret != ACL_SUCCESS) {
        return ret;
    }

    // 3. Allocate device memory
    size_t outputSize = static_cast<size_t>(n) * dtypeSize;
    void* devOut = nullptr;
    if (aclrtMalloc(&devOut, outputSize, ACL_MEM_MALLOC_HUGE_FIRST) != ACL_SUCCESS) {
        return ACL_ERROR;
    }

    // 4. Calculate tiling parameters (in element count)
    auto platform = platform_ascendc::PlatformAscendCManager::GetInstance();
    if (platform == nullptr) {
        aclrtFree(devOut);
        return ACL_ERROR;
    }
    StatelessRandomUniformV2TilingData tilingData;
    ret = CalcTilingData(n, dtypeSize, platform, tilingData);
    if (ret != ACL_SUCCESS) {
        aclrtFree(devOut);
        return ret;
    }
    PackSeedAndOffset(seed, offset, alg, tilingKey, tilingData);

    // 5. Launch kernel
    stateless_random_uniform_v2<<<tilingData.blockNum, nullptr, stream>>>(
        reinterpret_cast<uint8_t*>(devOut), tilingData);

    // 6. Wait for the kernel before copying back: the synchronous aclrtMemcpy is
    //    not ordered with respect to tasks on `stream`; copying earlier would race
    //    with the kernel and read uninitialized device memory
    if (stream != nullptr) {
        if (aclrtSynchronizeStream(stream) != ACL_SUCCESS) {
            aclrtFree(devOut);
            return ACL_ERROR;
        }
    } else {
        if (aclrtSynchronizeDevice() != ACL_SUCCESS) {
            aclrtFree(devOut);
            return ACL_ERROR;
        }
    }

    // 7. Copy result back to host
    if (aclrtMemcpy(output, outputSize, devOut, outputSize, ACL_MEMCPY_DEVICE_TO_HOST) != ACL_SUCCESS) {
        aclrtFree(devOut);
        return ACL_ERROR;
    }

    // 8. Free device memory
    aclrtFree(devOut);

    return ACL_SUCCESS;
}

extern "C" aclError _aclrandStatelessRandomUniformV2CounterUsage(uint64_t n, aclDataType dtype, uint64_t* counters)
{
    if (counters == nullptr || n == 0) {
        return ACL_ERROR_INVALID_PARAM;
    }

    uint32_t dtypeSize = 0;
    uint32_t tilingKey = 0;
    aclError ret = GetOutputSizeInfo(n, dtype, dtypeSize, tilingKey);
    if (ret != ACL_SUCCESS) {
        return ret;
    }

    auto platform = platform_ascendc::PlatformAscendCManager::GetInstance();
    if (platform == nullptr) {
        return ACL_ERROR;
    }
    StatelessRandomUniformV2TilingData tilingData;
    ret = CalcTilingData(n, dtypeSize, platform, tilingData);
    if (ret != ACL_SUCCESS) {
        return ret;
    }

    // 逐核复刻 kernel 的计数器消耗(Process/Skip): 每核先跳到 ceil(blockOffset / 4),
    // 其后每个 UB tile 的生成数量向上补齐到整个向量 repeat, 再按 genCount / 4 推进。
    // 取各核触达的最大计数器位置 —— 补齐使尾 tile 的消耗可能超过 ceil(n / 4),
    // 调用方按此值推进 offset 才能保证相邻调用的序列互不重叠。
    uint64_t maxCounterEnd = 0;
    for (uint32_t blockIdx = 0; blockIdx < tilingData.blockNum; ++blockIdx) {
        uint64_t blockOffset = static_cast<uint64_t>(tilingData.blockTilingSize) * blockIdx;
        uint64_t counterBase = CeilDiv(blockOffset, static_cast<uint64_t>(PHILOX_RESULT_ELEMENT_CNT));
        uint32_t currBlockTilingSize =
            (blockIdx == tilingData.blockNum - 1) ? tilingData.tailBlockTilingSize : tilingData.blockTilingSize;
        uint32_t ubTilingSize = std::min(tilingData.ubTilingSize, currBlockTilingSize);
        if (ubTilingSize == 0) {
            return ACL_ERROR;
        }
        uint32_t ubLoopCnt = CeilDiv(currBlockTilingSize, ubTilingSize);
        uint64_t used = 0;
        for (uint32_t idx = 0; idx < ubLoopCnt; ++idx) {
            uint32_t currUbTilingSize = ubTilingSize;
            if ((idx == ubLoopCnt - 1) && (currBlockTilingSize % ubTilingSize != 0)) {
                currUbTilingSize = currBlockTilingSize % ubTilingSize;
            }
            uint64_t genCount =
                CeilDiv(static_cast<uint64_t>(currUbTilingSize), static_cast<uint64_t>(PHILOX_GEN_ALIGN_ELEMENTS)) *
                PHILOX_GEN_ALIGN_ELEMENTS;
            used += genCount / PHILOX_RESULT_ELEMENT_CNT;
        }
        maxCounterEnd = std::max(maxCounterEnd, counterBase + used);
    }

    *counters = maxCounterEnd;
    return ACL_SUCCESS;
}
