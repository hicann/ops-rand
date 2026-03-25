/**
 * Copyright (c) 2026 Huawei Technologies Co., Ltd.
 * This program is free software, you can redistribute it and/or modify it under the terms and conditions of
 * CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */

#include "generator.h"

#include <new>

#include "cann_ops_rand.h"
#include "internal/ops_impl.h"

extern "C" {
namespace {

/**
 * @brief Convert aclError to aclrandStatus_t
 *
 * Maps ACL runtime error codes to ops-rand library error codes.
 *
 * Error code ranges in ACL:
 *   - 107xxx: Parameter/invalid state errors
 *   - 207xxx: Resource allocation/feature support errors
 *   - 507xxx: Internal/runtime errors
 *
 * @param aclRet ACL runtime error code
 * @return Corresponding aclrandStatus_t error code
 */
aclrandStatus_t toAclrandStatus(aclError aclRet) {
    if (aclRet == ACL_SUCCESS) {
        return ACLRAND_STATUS_SUCCESS;
    }

    // Parameter errors (107xxx): 107000-107039
    // Next range starts at 207000 (ACL_ERROR_RT_FEATURE_NOT_SUPPORT)
    if (aclRet >= ACL_ERROR_RT_PARAM_INVALID && aclRet < ACL_ERROR_RT_FEATURE_NOT_SUPPORT) {
        return ACLRAND_STATUS_OUT_OF_RANGE;
    }

    // Resource/feature errors (207xxx)
    switch (aclRet) {
        case ACL_ERROR_RT_MEMORY_ALLOCATION:
        case ACL_ERROR_RT_RESOURCE_ALLOC_FAIL:
            return ACLRAND_STATUS_ALLOCATION_FAILED;
        case ACL_ERROR_RT_FEATURE_NOT_SUPPORT:
            return ACLRAND_STATUS_TYPE_ERROR;
        default:
            break;
    }

    // Internal/runtime errors (507xxx)
    switch (aclRet) {
        case ACL_ERROR_RT_MODEL_EXECUTE:
        case ACL_ERROR_RT_AICORE_TIMEOUT:
        case ACL_ERROR_RT_AICORE_EXCEPTION:
        case ACL_ERROR_RT_STREAM_TASK_FULL:
            return ACLRAND_STATUS_LAUNCH_FAILURE;
        default:
            break;
    }

    // Other unknown errors
    return ACLRAND_STATUS_INTERNAL_ERROR;
}

aclrandStatus_t aclrandCreateGenerator(aclrandGenerator_t *generator, aclrandRngType_t rng_type) {
    if (generator == nullptr) {
        return ACLRAND_STATUS_ALLOCATION_FAILED;
    }
    opsrand::Generator *g = nullptr;
    try {
        g = new opsrand::Generator();
    } catch (const std::bad_alloc &) {
        return ACLRAND_STATUS_ALLOCATION_FAILED;
    }
    g->seed = 0;
    g->rng_type = static_cast<int>(rng_type);
    g->offset = 0;
    g->stream_id = 0;
    *generator = reinterpret_cast<aclrandGenerator_t>(g);
    return ACLRAND_STATUS_SUCCESS;
}

aclrandStatus_t aclrandSetGeneratorSeed(aclrandGenerator_t generator, uint64_t seed) {
    if (generator == nullptr) {
        return ACLRAND_STATUS_NOT_INITIALIZED;
    }
    opsrand::Generator *g = reinterpret_cast<opsrand::Generator*>(generator);
    g->seed = seed;
    return ACLRAND_STATUS_SUCCESS;
}

aclrandStatus_t aclrandSetGeneratorOffset(aclrandGenerator_t generator, size_t offset) {
    if (generator == nullptr) {
        return ACLRAND_STATUS_NOT_INITIALIZED;
    }
    opsrand::Generator *g = reinterpret_cast<opsrand::Generator*>(generator);
    g->offset = offset;
    return ACLRAND_STATUS_SUCCESS;
}

aclrandStatus_t aclrandSetGeneratorStream(aclrandGenerator_t generator, aclrtStream stream_id) {
    if (generator == nullptr) {
        return ACLRAND_STATUS_NOT_INITIALIZED;
    }
    opsrand::Generator *g = reinterpret_cast<opsrand::Generator*>(generator);
    g->stream_id = stream_id;
    return ACLRAND_STATUS_SUCCESS;
}

aclrandStatus_t aclrandDestroyGenerator(aclrandGenerator_t generator) {
    if (generator == nullptr) {
        return ACLRAND_STATUS_NOT_INITIALIZED;
    }
    opsrand::Generator *g = reinterpret_cast<opsrand::Generator*>(generator);
    delete g;
    return ACLRAND_STATUS_SUCCESS;
}

aclrandStatus_t aclrandGenerateUniform(aclrandGenerator_t generator, float *output, size_t n) {
    if (generator == nullptr || output == nullptr || n == 0) {
        return ACLRAND_STATUS_INTERNAL_ERROR;
    }

    opsrand::Generator *g = reinterpret_cast<opsrand::Generator*>(generator);

    aclError ret;
    // 当前算子只实现 Philox 算法，alg 参数预留未使用
    constexpr int32_t ALG_PHILOX = 1;

    switch (g->rng_type) {
        case ACLRAND_RNG_PSEUDO_DEFAULT:
        case ACLRAND_RNG_PSEUDO_PHILOX4_32_10:
            ret = _aclrandStatelessRandomUniformV2(
                g->seed, g->offset, ALG_PHILOX, output, n, ACL_FLOAT);
            break;
        default:
            return ACLRAND_STATUS_TYPE_ERROR;
    }

    return toAclrandStatus(ret);
}

aclrandStatus_t aclrandGetVersion(size_t* version) {
    if (version == nullptr) {
        return ACLRAND_STATUS_INTERNAL_ERROR;
    }
    *version = ACLRAND_VERSION;
    return ACLRAND_STATUS_SUCCESS;
}

} // namespace

} // extern "C"
