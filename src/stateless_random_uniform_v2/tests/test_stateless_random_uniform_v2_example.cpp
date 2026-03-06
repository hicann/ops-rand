/**
 * Copyright (c) 2026 Huawei Technologies Co., Ltd.
 * This program is free software, you can redistribute it and/or modify it under the terms and conditions of
 * CANN Open Software License Agreement License Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR. FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */

#include <iostream>
#include <vector>
#include <cstdint>
#include "acl/acl.h"
#include "../../../include/cann_ops_rand.h"

#define CHECK_RET(cond, return_expr) \
    do {                             \
        if (!(cond)) {               \
            return_expr;             \
        }                            \
    } while (0)

#define LOG_PRINT(message, ...)         \
    do {                                \
        printf(message, ##__VA_ARGS__); \
    } while (0)

aclError aclrandStatelessRandomUniformV2(uint64_t seed, uint64_t offset, int32_t alg, float *output, uint64_t n);

int main()
{
    int32_t deviceId = 0;
    aclError ret = aclInit(nullptr);
    CHECK_RET(ret == ACL_SUCCESS, LOG_PRINT("aclInit failed. ERROR: %d\n", ret); return ret);
    
    ret = aclrtSetDevice(deviceId);
    CHECK_RET(ret == ACL_SUCCESS, LOG_PRINT("aclrtSetDevice failed. ERROR: %d\n", ret); return ret);
    
    uint64_t seed = 12345;
    uint64_t offset = 0;
    int32_t alg = 1;
    uint64_t n = 2048;
    std::vector<float> output(n);
    
    ret = aclrandStatelessRandomUniformV2(seed, offset, alg, output.data(), n);
    CHECK_RET(ret == ACL_SUCCESS, LOG_PRINT("aclrandStatelessRandomUniformV2 failed. ERROR: %d\n", ret); return ret);
    
    LOG_PRINT("Generated %lu random numbers:\n", n);
    for (uint64_t i = 0; i < n; i++) {
        LOG_PRINT("output[%lu] = %f\n", i, output[i]);
    }
    
    ret = aclrtResetDevice(deviceId);
    CHECK_RET(ret == ACL_SUCCESS, LOG_PRINT("aclrtResetDevice failed. ERROR: %d\n", ret));
    
    ret = aclFinalize();
    CHECK_RET(ret == ACL_SUCCESS, LOG_PRINT("aclFinalize failed. ERROR: %d\n", ret));
    
    LOG_PRINT("Test passed!\n");
    return 0;
}
