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
#include "acl/acl.h"

/**
 * @brief API version macros
 *
 * This version represents the ops-rand library API version, returned by aclrandGetVersion().
 * It is independent from the CANN package version defined in version.cmake
 * (used for release and dependency management).
 *
 * Version encoding: MAJOR * 10000 + MINOR * 100 + PATCH
 * Examples: 1.0.0 -> 10000, 1.2.3 -> 10203
 */
#define ACLRAND_VERSION_MAJOR 1
#define ACLRAND_VERSION_MINOR 0
#define ACLRAND_VERSION_PATCH 0
#define ACLRAND_VERSION ((ACLRAND_VERSION_MAJOR * 10000) + (ACLRAND_VERSION_MINOR * 100) + ACLRAND_VERSION_PATCH)

/* Export macro definition */
#if defined(_WIN32) || defined(__CYGWIN__)
#define ACLRAND_API __declspec(dllexport)
#else
#define ACLRAND_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct aclrandGenerator_st* aclrandGenerator_t; /* opaque handle */

/**
 * @brief Error codes for ops-rand library operations
 *
 * Defines all possible return status codes for ops-rand library functions.
*/
typedef enum
{
    /* Success */
    ACLRAND_STATUS_SUCCESS = 0,                      /**< Operation successful */

    /* Parameter/State errors (100 series) - caller issues detectable before execution */
    ACLRAND_STATUS_VERSION_MISMATCH = 100,           /**< Header file and shared library version mismatch */
    ACLRAND_STATUS_NOT_INITIALIZED = 101,            /**< Library not initialized */
    ACLRAND_STATUS_ALLOCATION_FAILED = 102,          /**< Memory allocation failed */
    ACLRAND_STATUS_TYPE_ERROR = 103,                 /**< RNG type not supported */
    ACLRAND_STATUS_OUT_OF_RANGE = 104,               /**< Parameter out of valid range */
    ACLRAND_STATUS_LENGTH_NOT_MULTIPLE = 105,        /**< Output length not as required */
    ACLRAND_STATUS_DOUBLE_PRECISION_REQUIRED = 106,  /**< Double precision not supported on device */

    /* Runtime errors (200 series) - execution environment issues */
    ACLRAND_STATUS_LAUNCH_FAILURE = 201,             /**< Kernel launch failed */
    ACLRAND_STATUS_PREEXISTING_FAILURE = 202,        /**< Pre-existing error on device */
    ACLRAND_STATUS_INITIALIZATION_FAILED = 203,      /**< Initialization failed */
    ACLRAND_STATUS_ARCH_MISMATCH = 204,              /**< Architecture mismatch */

    /* Internal error */
    ACLRAND_STATUS_INTERNAL_ERROR = 999,             /**< Internal error */

    /* Extended error codes */
    ACLRAND_STATUS_NOT_IMPLEMENTED = 1000,           /**< Feature not implemented yet */
} aclrandStatus_t;

/**
 * @brief Random number generator types
 *
 * Defines supported pseudo-random and quasi-random number generator algorithms.
 *
 * Pseudo-random generators (0-99):
 *   - DEFAULT: Default algorithm (currently Philox)
 *   - XORWOW: XORWOW algorithm (pending)
 *   - MRG32K3A: MRG32K3A algorithm (pending)
 *   - MTGP32: MTGP32 algorithm (pending)
 *   - PHILOX4_32_10: Philox 4x32 algorithm with 10 rounds (implemented)
 *   - MT19937: Mersenne Twister algorithm (pending)
 *
 * Quasi-random generators (100-199):
 *   - SOBOL32: 32-bit Sobol sequence (pending)
 *   - SCRAMBLED_SOBOL32: Scrambled Sobol32 (pending)
 *   - SOBOL64: 64-bit Sobol sequence (pending)
 *   - SCRAMBLED_SOBOL64: Scrambled Sobol64 (pending)
 */
typedef enum
{
    ACLRAND_RNG_PSEUDO_DEFAULT = 0,       /**< Default pseudo-random algorithm */
    ACLRAND_RNG_PSEUDO_XORWOW = 1,        /**< XORWOW algorithm */
    ACLRAND_RNG_PSEUDO_MRG32K3A = 2,      /**< MRG32K3A algorithm */
    ACLRAND_RNG_PSEUDO_MTGP32 = 3,        /**< MTGP32 algorithm */
    ACLRAND_RNG_PSEUDO_PHILOX4_32_10 = 4, /**< Philox 4x32 algorithm with 10 rounds */
    ACLRAND_RNG_PSEUDO_MT19937 = 5,       /**< Mersenne Twister algorithm */
    ACLRAND_RNG_QUASI_DEFAULT = 100,      /**< Default quasi-random algorithm */
    ACLRAND_RNG_QUASI_SOBOL32 = 101,      /**< 32-bit Sobol sequence */
    ACLRAND_RNG_QUASI_SCRAMBLED_SOBOL32 = 102, /**< Scrambled Sobol32 */
    ACLRAND_RNG_QUASI_SOBOL64 = 103,      /**< 64-bit Sobol sequence */
    ACLRAND_RNG_QUASI_SCRAMBLED_SOBOL64 = 104, /**< Scrambled Sobol64 */
} aclrandRngType_t;

/**
 * @brief Get library version
 *
 * Returns the version number of the ops-rand library.
 *
 * @param[out] version Pointer to store version number. The version is encoded as:
 *                     MAJOR * 10000 + MINOR * 100 + PATCH
 *                     e.g., version 1.2.3 is encoded as 10203.
 *
 * @return ACLRAND_STATUS_SUCCESS on success, ACLRAND_STATUS_INTERNAL_ERROR if version is NULL.
 */
ACLRAND_API aclrandStatus_t aclrandGetVersion(size_t* version);

/**
 * @brief Create random number generator
 *
 * Creates a new random number generator of the specified type.
 *
 * @param[out] generator Pointer to generator handle to be created.
 * @param[in]  rng_type  Type of RNG algorithm to use (see ::aclrandRngType_t).
 *
 * @return ACLRAND_STATUS_SUCCESS on success, ACLRAND_STATUS_ALLOCATION_FAILED if generator is NULL
 *         or memory allocation fails.
 *
 * @note Currently only ACLRAND_RNG_PSEUDO_DEFAULT and ACLRAND_RNG_PSEUDO_PHILOX4_32_10
 *       are fully supported. Other types return ACLRAND_STATUS_TYPE_ERROR
 *       when calling generate functions.
 */
ACLRAND_API aclrandStatus_t aclrandCreateGenerator(aclrandGenerator_t* generator, aclrandRngType_t rng_type);

/**
 * @brief Set generator seed
 *
 * Sets the seed value for the random number generator.
 *
 * @param[in] generator Generator handle.
 * @param[in] seed     Seed value for random number generation.
 *
 * @return ACLRAND_STATUS_SUCCESS on success, ACLRAND_STATUS_NOT_INITIALIZED if generator is NULL.
 */
ACLRAND_API aclrandStatus_t aclrandSetGeneratorSeed(aclrandGenerator_t generator, uint64_t seed);

/**
 * @brief Set generator offset
 *
 * Sets the starting offset in the random number sequence.
 *
 * @param[in] generator Generator handle.
 * @param[in] offset   Starting offset in the random sequence.
 *
 * @return ACLRAND_STATUS_SUCCESS on success, ACLRAND_STATUS_NOT_INITIALIZED if generator is NULL.
 */
ACLRAND_API aclrandStatus_t aclrandSetGeneratorOffset(aclrandGenerator_t generator, size_t offset);

/**
 * @brief Set generator stream
 *
 * Sets the ACL stream for asynchronous operations.
 *
 * @param[in] generator Generator handle.
 * @param[in] stream    ACL runtime stream handle.
 *
 * @return ACLRAND_STATUS_SUCCESS on success, ACLRAND_STATUS_NOT_INITIALIZED if generator is NULL.
 */
ACLRAND_API aclrandStatus_t aclrandSetGeneratorStream(aclrandGenerator_t generator, aclrtStream stream);

/**
 * @brief Destroy generator
 *
 * Releases resources associated with the generator handle.
 *
 * @param[in] generator Generator handle to destroy.
 *
 * @return ACLRAND_STATUS_SUCCESS on success, ACLRAND_STATUS_NOT_INITIALIZED if generator is NULL.
 */
ACLRAND_API aclrandStatus_t aclrandDestroyGenerator(aclrandGenerator_t generator);

/**
 * @brief Generate uniform random numbers
 *
 * Generates uniformly distributed random numbers in the range [0, 1).
 *
 * @param[in]  generator Generator handle.
 * @param[out] output    Output array (host memory) to store random numbers.
 * @param[in]  n         Number of random numbers to generate.
 *
 * @return ACLRAND_STATUS_SUCCESS on success, ACLRAND_STATUS_TYPE_ERROR if
 *         the RNG type is not supported, or other error codes on failure.
 *
 * @note Currently supports ACLRAND_RNG_PSEUDO_DEFAULT and ACLRAND_RNG_PSEUDO_PHILOX4_32_10.
 */
ACLRAND_API aclrandStatus_t aclrandGenerateUniform(aclrandGenerator_t generator, float* output, size_t n);

#ifdef __cplusplus
}
#endif
