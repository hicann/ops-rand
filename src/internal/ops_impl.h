/**
 * Copyright (c) 2026 Huawei Technologies Co., Ltd.
 * This program is free software, you can redistribute it and/or modify it under the terms and conditions of
 * CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS BASIS", WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */

/**
 * @file ops_impl.h
 * @brief Internal operator implementations (not exposed in public API)
 *
 * This header declares internal operator functions that are used by the
 * public API layer (lib/generator.cpp) but are not exposed to end users.
 */

#pragma once

#include "acl/acl.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Internal stateless random uniform distribution v2
 *
 * Generates uniformly distributed random numbers in range [0, 1)
 * using Philox algorithm. This is an internal function called by
 * aclrandGenerateUniform().
 *
 * @param seed    Random seed for Philox RNG
 * @param offset  Starting offset in random sequence
 * @param alg     Algorithm identifier (reserved for future use)
 * @param output  Output array pointer (host memory)
 * @param n       Number of random numbers to generate
 * @param dtype   Data type of output (ACL_FLOAT, ACL_FLOAT16, or ACL_BF16)
 *
 * @return ACL_SUCCESS on success, ACL_ERROR otherwise
 */
aclError aclrandStatelessRandomUniformV2(
    uint64_t seed, uint64_t offset, int32_t alg, void* output, uint64_t n, aclDataType dtype);

/* Add more internal operator declarations here as needed:
 *
 * aclError aclrandStatelessRandomNormalV2(...);
 * aclError aclrandStatelessRandIntV2(...);
 */

#ifdef __cplusplus
}
#endif
