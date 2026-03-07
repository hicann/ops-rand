/**
* Copyright (c) 2026 Huawei Technologies Co., Ltd.
* This program is free software, you can redistribute it and/or modify it under the terms and conditions of
* CANN Open Software License Agreement Version 2.0 (the "License").
* Please refer to the License for details. You may not use this file except in compliance with the License.
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
* INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
* See LICENSE in the root of the software repository for the full text of the License.
*/

#pragma once
#include <cstdint>

/* 导出宏定义 */
#if defined(_WIN32) || defined(__CYGWIN__)
    #define ACLRAND_API __declspec(dllexport)
#else
    #define ACLRAND_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct aclrandGenerator_st* aclrandGenerator_t; /* opaque handle */

typedef enum {
    ACLRAND_RNG_PSEUDO_DEFAULT = 0,
} aclrandRngType_t;

/* Create a generator handle. Output parameter `generator` receives the opaque handle. */
ACLRAND_API int aclrandCreateGenerator(aclrandGenerator_t *generator, aclrandRngType_t rng_type);

/* Set generator parameters */
ACLRAND_API int aclrandSetGeneratorSeed(aclrandGenerator_t generator, uint64_t seed);
ACLRAND_API int aclrandSetGeneratorOffset(aclrandGenerator_t generator, size_t offset);
ACLRAND_API int aclrandSetGeneratorStream(aclrandGenerator_t generator, aclrtStream stream);

/* Destroy the generator handle */
ACLRAND_API int aclrandDestroyGenerator(aclrandGenerator_t generator);

/* Generate uniform random numbers */
ACLRAND_API aclError aclrandGenerateUniform(aclrandGenerator_t *generator, float *output, uint64_t n);

/* Stateless random uniform distribution v2 */
ACLRAND_API aclError aclrandStatelessRandomUniformV2(uint64_t seed, uint64_t offset, int32_t alg, float *output, uint64_t n);

#ifdef __cplusplus
}
#endif
