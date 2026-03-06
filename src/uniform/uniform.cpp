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

#include "arch35/uniform.h"
#include "../common/utils.h"
#include "../common/generator.h"
#include "platform/platform_info.h"
#include "tiling/platform/platform_ascendc.h"
#include "log/log.h"
#include "cann_ops_rand.h"


using namespace StatelessRandom;
using namespace opsrand;

namespace {
#define FLOAT_TILING_KEY 101
#define FLOAT16_TILING_KEY 102
#define BFLOAT16_TILING_KEY 103
} // namespace

constexpr uint32_t CORE_ALIGN_SIZE = 512;
constexpr uint32_t MIN_TILING_SIZE = 256;
constexpr uint32_t BLOCK_SIZE_BYTES = 32;
template <typename T1, typename T2>
inline T1 CeilDiv(const T1 a, const T2 b)
{
    if (b == 0) {
        return 0;
    }
    return (a + b - 1) / b;
}


extern "C" __global__ __aicore__ void stateless_random_uniform_v2(GM_ADDR y,  StatelessRandomUniformV2TilingData tilingData)
{
    TPipe pipe;
    StatelessRandomUniformV2<float> op;
    op.Init(y, &tilingData, &pipe);
    op.Process();   

}

aclError aclrandGenerateUniform(aclrandGenerator_t *generator, float *output, uint64_t n)
{
    if (generator == nullptr || output == nullptr || n == 0) {
        return ACL_ERROR;
    }

    opsrand::Generator *g = reinterpret_cast<opsrand::Generator*>(generator);

    auto outputDtypeSize = sizeof(float);

    size_t outputSize_ = static_cast<size_t>(n) * outputDtypeSize;

     // Allocate device memory for output
     void *dev_out = nullptr;
     if (aclrtMalloc(&dev_out, outputSize_, ACL_MEM_MALLOC_HUGE_FIRST) != ACL_SUCCESS) {
         return ACL_ERROR;
     }
    auto ascendcPlatform = platform_ascendc::PlatformAscendCManager::GetInstance();
  
    StatelessRandomUniformV2TilingData *tilingData = new StatelessRandomUniformV2TilingData();
    auto coreNum_ = ascendcPlatform->GetCoreNumAiv();
    auto coreAlignFactor = CORE_ALIGN_SIZE / outputDtypeSize;
    auto blockFactor = CeilDiv(outputSize_, coreNum_);
    auto blockAlignFactor = CeilDiv(blockFactor, coreAlignFactor) * coreAlignFactor;
    auto minTilingSize = MIN_TILING_SIZE;
    tilingData->blockTilingSize = std::max(static_cast<uint32_t>(blockAlignFactor), minTilingSize);
    tilingData->blockNum = CeilDiv(outputSize_, tilingData->blockTilingSize);
    tilingData->tailBlockTilingSize = outputSize_ - tilingData->blockTilingSize * (tilingData->blockNum - 1);

    uint64_t ubSize = 0;
    ascendcPlatform->GetCoreMemSize(platform_ascendc::CoreMemType::UB, ubSize);
    auto coefVal = 2;
    auto quarterUbSize = ubSize / (2 + coefVal);
    auto alignFactor = BLOCK_SIZE_BYTES / outputDtypeSize;
    tilingData->ubTilingSize = CeilDiv(quarterUbSize / outputDtypeSize, alignFactor) * alignFactor;
    tilingData->alg = 1;
    tilingData->counter[0] = static_cast<uint32_t>(g->offset);
    tilingData->counter[1] = static_cast<uint32_t>(g->offset >> 32); // 32 for upper 32 bits
    tilingData->key[0] = static_cast<uint32_t>(g->seed);
    tilingData->key[1] = static_cast<uint32_t>(g->seed >> 32); // 32 for lower 32 bits
    
    // Call host-side launcher (must be implemented to actually launch kernel)
    stateless_random_uniform_v2<<<tilingData->blockNum, nullptr, g->stream_id>>>((__gm__ uint8_t *)dev_out, *tilingData);

    // Copy device output back to host
    if (aclrtMemcpy(output, outputSize_, dev_out, outputSize_, ACL_MEMCPY_DEVICE_TO_HOST) != ACL_SUCCESS) {
        aclrtFree(dev_out);
        return ACL_ERROR;
    }

    // Free device memory
    aclrtFree(dev_out);

    return ACL_SUCCESS;
}

