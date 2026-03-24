/**
 * Copyright (c) 2026 Huawei Technologies Co., Ltd.
 * This program is free software; you can redistribute it and/or modify it under the terms and conditions of
 * CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */

/**
 * @file generator_test.cpp
 * @brief Generator API unit tests
 */

#include "generator_test.h"
#include "cann_ops_rand.h"
#include <iostream>

namespace GeneratorTest {

// Test create generator
void test_create_generator(aclrtStream stream, OpsRandTest::TestStats& stats)
{
    TEST_CASE_BEGIN("test_create_generator");

    aclrandGenerator_t generator;
    aclrandStatus_t ret = aclrandCreateGenerator(&generator, ACLRAND_RNG_PSEUDO_DEFAULT);
    TEST_ASSERT(stats, ret == ACLRAND_STATUS_SUCCESS, "aclrandCreateGenerator failed");
    TEST_ASSERT(stats, generator != nullptr, "generator should not be null");

    ret = aclrandDestroyGenerator(generator);
    TEST_ASSERT(stats, ret == ACLRAND_STATUS_SUCCESS, "aclrandDestroyGenerator failed");

    TEST_CASE_PASS(stats, "test_create_generator");
}

// Test create with null pointer
void test_create_null_generator(aclrtStream stream, OpsRandTest::TestStats& stats)
{
    TEST_CASE_BEGIN("test_create_null_generator");

    aclrandStatus_t ret = aclrandCreateGenerator(nullptr, ACLRAND_RNG_PSEUDO_DEFAULT);
    TEST_ASSERT(stats, ret != ACLRAND_STATUS_SUCCESS, "aclrandCreateGenerator with null should fail");

    TEST_CASE_PASS(stats, "test_create_null_generator");
}

// Test set seed
void test_set_seed(aclrtStream stream, OpsRandTest::TestStats& stats)
{
    TEST_CASE_BEGIN("test_set_seed");

    aclrandGenerator_t generator;
    aclrandStatus_t ret = aclrandCreateGenerator(&generator, ACLRAND_RNG_PSEUDO_DEFAULT);
    TEST_ASSERT(stats, ret == ACLRAND_STATUS_SUCCESS, "aclrandCreateGenerator failed");

    ret = aclrandSetGeneratorSeed(generator, 12345);
    TEST_ASSERT(stats, ret == ACLRAND_STATUS_SUCCESS, "aclrandSetGeneratorSeed failed");

    ret = aclrandSetGeneratorSeed(nullptr, 12345);
    TEST_ASSERT(stats, ret != ACLRAND_STATUS_SUCCESS, "aclrandSetGeneratorSeed with null should fail");

    ret = aclrandDestroyGenerator(generator);
    TEST_ASSERT(stats, ret == ACLRAND_STATUS_SUCCESS, "aclrandDestroyGenerator failed");

    TEST_CASE_PASS(stats, "test_set_seed");
}

// Test set offset
void test_set_offset(aclrtStream stream, OpsRandTest::TestStats& stats)
{
    TEST_CASE_BEGIN("test_set_offset");

    aclrandGenerator_t generator;
    aclrandStatus_t ret = aclrandCreateGenerator(&generator, ACLRAND_RNG_PSEUDO_DEFAULT);
    TEST_ASSERT(stats, ret == ACLRAND_STATUS_SUCCESS, "aclrandCreateGenerator failed");

    ret = aclrandSetGeneratorOffset(generator, 100);
    TEST_ASSERT(stats, ret == ACLRAND_STATUS_SUCCESS, "aclrandSetGeneratorOffset failed");

    ret = aclrandSetGeneratorOffset(nullptr, 100);
    TEST_ASSERT(stats, ret != ACLRAND_STATUS_SUCCESS, "aclrandSetGeneratorOffset with null should fail");

    ret = aclrandDestroyGenerator(generator);
    TEST_ASSERT(stats, ret == ACLRAND_STATUS_SUCCESS, "aclrandDestroyGenerator failed");

    TEST_CASE_PASS(stats, "test_set_offset");
}

// Test set stream
void test_set_stream(aclrtStream stream, OpsRandTest::TestStats& stats)
{
    TEST_CASE_BEGIN("test_set_stream");

    aclrandGenerator_t generator;
    aclrandStatus_t ret = aclrandCreateGenerator(&generator, ACLRAND_RNG_PSEUDO_DEFAULT);
    TEST_ASSERT(stats, ret == ACLRAND_STATUS_SUCCESS, "aclrandCreateGenerator failed");

    ret = aclrandSetGeneratorStream(generator, stream);
    TEST_ASSERT(stats, ret == ACLRAND_STATUS_SUCCESS, "aclrandSetGeneratorStream failed");

    ret = aclrandSetGeneratorStream(nullptr, stream);
    TEST_ASSERT(stats, ret != ACLRAND_STATUS_SUCCESS, "aclrandSetGeneratorStream with null generator should fail");

    ret = aclrandDestroyGenerator(generator);
    TEST_ASSERT(stats, ret == ACLRAND_STATUS_SUCCESS, "aclrandDestroyGenerator failed");

    TEST_CASE_PASS(stats, "test_set_stream");
}

// Test destroy null generator
void test_destroy_null_generator(aclrtStream stream, OpsRandTest::TestStats& stats)
{
    TEST_CASE_BEGIN("test_destroy_null_generator");

    aclrandStatus_t ret = aclrandDestroyGenerator(nullptr);
    TEST_ASSERT(stats, ret != ACLRAND_STATUS_SUCCESS, "aclrandDestroyGenerator with null should fail");

    TEST_CASE_PASS(stats, "test_destroy_null_generator");
}

// Test aclrandGenerateUniform basic functionality
void test_generate_uniform(aclrtStream stream, OpsRandTest::TestStats& stats)
{
    TEST_CASE_BEGIN("test_generate_uniform");

    // Create generator
    aclrandGenerator_t generator;
    aclrandStatus_t ret = aclrandCreateGenerator(&generator, ACLRAND_RNG_PSEUDO_DEFAULT);
    TEST_ASSERT(stats, ret == ACLRAND_STATUS_SUCCESS, "aclrandCreateGenerator failed");

    // Set parameters
    ret = aclrandSetGeneratorSeed(generator, 12345);
    TEST_ASSERT(stats, ret == ACLRAND_STATUS_SUCCESS, "aclrandSetGeneratorSeed failed");

    ret = aclrandSetGeneratorOffset(generator, 0);
    TEST_ASSERT(stats, ret == ACLRAND_STATUS_SUCCESS, "aclrandSetGeneratorOffset failed");

    // Generate random numbers
    const uint64_t n = 100;
    float output[n];
    aclrandStatus_t status = aclrandGenerateUniform(generator, output, n);
    TEST_ASSERT(stats, status == ACLRAND_STATUS_SUCCESS, "aclrandGenerateUniform failed");

    // Verify all values are in [0, 1) range
    bool all_in_range = true;
    for (uint64_t i = 0; i < n; ++i) {
        if (output[i] < 0.0f || output[i] >= 1.0f) {
            all_in_range = false;
            break;
        }
    }
    TEST_ASSERT(stats, all_in_range, "values not in [0, 1) range");

    ret = aclrandDestroyGenerator(generator);
    TEST_ASSERT(stats, ret == ACLRAND_STATUS_SUCCESS, "aclrandDestroyGenerator failed");

    TEST_CASE_PASS(stats, "test_generate_uniform");
}

// Test aclrandGenerateUniform with null parameters
void test_generate_uniform_null(aclrtStream stream, OpsRandTest::TestStats& stats)
{
    TEST_CASE_BEGIN("test_generate_uniform_null");

    aclrandGenerator_t generator;
    aclrandStatus_t ret = aclrandCreateGenerator(&generator, ACLRAND_RNG_PSEUDO_DEFAULT);
    TEST_ASSERT(stats, ret == ACLRAND_STATUS_SUCCESS, "aclrandCreateGenerator failed");

    float output[10];

    // Null generator pointer
    aclrandStatus_t status = aclrandGenerateUniform(nullptr, output, 10);
    TEST_ASSERT(stats, status != ACLRAND_STATUS_SUCCESS, "aclrandGenerateUniform with null generator ptr should fail");

    // Null output
    status = aclrandGenerateUniform(generator, nullptr, 10);
    TEST_ASSERT(stats, status != ACLRAND_STATUS_SUCCESS, "aclrandGenerateUniform with null output should fail");

    // Zero size
    status = aclrandGenerateUniform(generator, output, 0);
    TEST_ASSERT(stats, status != ACLRAND_STATUS_SUCCESS, "aclrandGenerateUniform with zero n should fail");

    ret = aclrandDestroyGenerator(generator);
    TEST_ASSERT(stats, ret == ACLRAND_STATUS_SUCCESS, "aclrandDestroyGenerator failed");

    TEST_CASE_PASS(stats, "test_generate_uniform_null");
}

// Test aclrandGenerateUniform reproducibility
void test_generate_uniform_reproducibility(aclrtStream stream, OpsRandTest::TestStats& stats)
{
    TEST_CASE_BEGIN("test_generate_uniform_reproducibility");

    const uint64_t n = 50;
    float output1[n];
    float output2[n];

    // First generation
    aclrandGenerator_t gen1;
    aclrandStatus_t ret = aclrandCreateGenerator(&gen1, ACLRAND_RNG_PSEUDO_DEFAULT);
    TEST_ASSERT(stats, ret == ACLRAND_STATUS_SUCCESS, "aclrandCreateGenerator failed");
    ret = aclrandSetGeneratorSeed(gen1, 99999);
    TEST_ASSERT(stats, ret == ACLRAND_STATUS_SUCCESS, "aclrandSetGeneratorSeed failed");
    ret = aclrandSetGeneratorOffset(gen1, 0);
    TEST_ASSERT(stats, ret == ACLRAND_STATUS_SUCCESS, "aclrandSetGeneratorOffset failed");
    aclrandStatus_t status = aclrandGenerateUniform(gen1, output1, n);
    TEST_ASSERT(stats, status == ACLRAND_STATUS_SUCCESS, "aclrandGenerateUniform failed");
    ret = aclrandDestroyGenerator(gen1);
    TEST_ASSERT(stats, ret == ACLRAND_STATUS_SUCCESS, "aclrandDestroyGenerator failed");

    // Second generation (same parameters)
    aclrandGenerator_t gen2;
    ret = aclrandCreateGenerator(&gen2, ACLRAND_RNG_PSEUDO_DEFAULT);
    TEST_ASSERT(stats, ret == ACLRAND_STATUS_SUCCESS, "aclrandCreateGenerator failed");
    ret = aclrandSetGeneratorSeed(gen2, 99999);
    TEST_ASSERT(stats, ret == ACLRAND_STATUS_SUCCESS, "aclrandSetGeneratorSeed failed");
    ret = aclrandSetGeneratorOffset(gen2, 0);
    TEST_ASSERT(stats, ret == ACLRAND_STATUS_SUCCESS, "aclrandSetGeneratorOffset failed");
    status = aclrandGenerateUniform(gen2, output2, n);
    TEST_ASSERT(stats, status == ACLRAND_STATUS_SUCCESS, "aclrandGenerateUniform failed");
    ret = aclrandDestroyGenerator(gen2);
    TEST_ASSERT(stats, ret == ACLRAND_STATUS_SUCCESS, "aclrandDestroyGenerator failed");

    // Verify both results are identical
    TEST_ASSERT_ARRAY_EQ(stats, output1, output2, n, "results not reproducible");

    TEST_CASE_PASS(stats, "test_generate_uniform_reproducibility");
}

// Test unsupported RNG types
void test_generate_uniform_unsupported_rng(aclrtStream stream, OpsRandTest::TestStats& stats)
{
    TEST_CASE_BEGIN("test_generate_uniform_unsupported_rng");

    float output[10];
    aclrandStatus_t status;

    // Test XORWOW (not supported)
    {
        aclrandGenerator_t gen;
        aclrandStatus_t ret = aclrandCreateGenerator(&gen, ACLRAND_RNG_PSEUDO_XORWOW);
        TEST_ASSERT(stats, ret == ACLRAND_STATUS_SUCCESS, "aclrandCreateGenerator failed");
        status = aclrandGenerateUniform(gen, output, 10);
        TEST_ASSERT(stats, status == ACLRAND_STATUS_TYPE_ERROR,
                    "XORWOW should return TYPE_NOT_SUPPORTED");
        aclrandDestroyGenerator(gen);
    }

    // Test MRG32K3A (not supported)
    {
        aclrandGenerator_t gen;
        aclrandStatus_t ret = aclrandCreateGenerator(&gen, ACLRAND_RNG_PSEUDO_MRG32K3A);
        TEST_ASSERT(stats, ret == ACLRAND_STATUS_SUCCESS, "aclrandCreateGenerator failed");
        status = aclrandGenerateUniform(gen, output, 10);
        TEST_ASSERT(stats, status == ACLRAND_STATUS_TYPE_ERROR,
                    "MRG32K3A should return TYPE_NOT_SUPPORTED");
        aclrandDestroyGenerator(gen);
    }

    // Test quasi-random SOBOL32 (not supported)
    {
        aclrandGenerator_t gen;
        aclrandStatus_t ret = aclrandCreateGenerator(&gen, ACLRAND_RNG_QUASI_SOBOL32);
        TEST_ASSERT(stats, ret == ACLRAND_STATUS_SUCCESS, "aclrandCreateGenerator failed");
        status = aclrandGenerateUniform(gen, output, 10);
        TEST_ASSERT(stats, status == ACLRAND_STATUS_TYPE_ERROR,
                    "SOBOL32 should return TYPE_NOT_SUPPORTED");
        aclrandDestroyGenerator(gen);
    }

    // Test PHILOX (supported)
    {
        aclrandGenerator_t gen;
        aclrandStatus_t ret = aclrandCreateGenerator(&gen, ACLRAND_RNG_PSEUDO_PHILOX4_32_10);
        TEST_ASSERT(stats, ret == ACLRAND_STATUS_SUCCESS, "aclrandCreateGenerator failed");
        status = aclrandGenerateUniform(gen, output, 10);
        TEST_ASSERT(stats, status == ACLRAND_STATUS_SUCCESS,
                    "PHILOX4_32_10 should succeed");
        aclrandDestroyGenerator(gen);
    }

    TEST_CASE_PASS(stats, "test_generate_uniform_unsupported_rng");
}

void run_all_tests(aclrtStream stream, OpsRandTest::TestStats& stats)
{
    test_create_generator(stream, stats);
    test_create_null_generator(stream, stats);
    test_set_seed(stream, stats);
    test_set_offset(stream, stats);
    test_set_stream(stream, stats);
    test_destroy_null_generator(stream, stats);
    test_generate_uniform(stream, stats);
    test_generate_uniform_null(stream, stats);
    test_generate_uniform_reproducibility(stream, stats);
    test_generate_uniform_unsupported_rng(stream, stats);
}

} // namespace GeneratorTest

// Auto-register to test framework
REGISTER_API_TEST(Generator)
