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

    ret = aclrandSetGeneratorSeed(generator, 12345);  // 12345为测试用的固定seed
    TEST_ASSERT(stats, ret == ACLRAND_STATUS_SUCCESS, "aclrandSetGeneratorSeed failed");

    ret = aclrandSetGeneratorSeed(nullptr, 12345);  // 12345为测试用的固定seed
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

    ret = aclrandSetGeneratorOffset(generator, 100);  // 100为测试用的固定offset
    TEST_ASSERT(stats, ret == ACLRAND_STATUS_SUCCESS, "aclrandSetGeneratorOffset failed");

    ret = aclrandSetGeneratorOffset(nullptr, 100);  // 100为测试用的固定offset
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
    ret = aclrandSetGeneratorSeed(generator, 12345);  // 12345为测试用的固定seed
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
    status = aclrandGenerateUniform(generator, nullptr, 10);  // 10为要生成的随机数个数
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

namespace {

// 单次生成 2n 个元素必须与同一 seed/offset 下连续两次 n 个元素的拼接逐位相同:
// 推进量少算会与第一段重叠, 多算会跳值, 两种情况都会失配
void AssertOffsetAdvanceMatchesConsumption(
    uint64_t seed, uint64_t offset, uint64_t n, const float* first, const float* second,
    OpsRandTest::TestStats& stats)
{
    std::vector<float> single(2 * n);
    std::vector<float> combined(2 * n);
    for (uint64_t i = 0; i < n; ++i) {
        combined[i] = first[i];
        combined[n + i] = second[i];
    }

    aclrandGenerator_t gen;
    aclrandStatus_t ret = aclrandCreateGenerator(&gen, ACLRAND_RNG_PSEUDO_DEFAULT);
    TEST_ASSERT(stats, ret == ACLRAND_STATUS_SUCCESS, "aclrandCreateGenerator failed");
    ret = aclrandSetGeneratorSeed(gen, seed);
    TEST_ASSERT(stats, ret == ACLRAND_STATUS_SUCCESS, "aclrandSetGeneratorSeed failed");
    ret = aclrandSetGeneratorOffset(gen, offset);
    TEST_ASSERT(stats, ret == ACLRAND_STATUS_SUCCESS, "aclrandSetGeneratorOffset failed");
    aclrandStatus_t status = aclrandGenerateUniform(gen, single.data(), 2 * n);
    TEST_ASSERT(stats, status == ACLRAND_STATUS_SUCCESS, "single 2n aclrandGenerateUniform failed");
    ret = aclrandDestroyGenerator(gen);
    TEST_ASSERT(stats, ret == ACLRAND_STATUS_SUCCESS, "aclrandDestroyGenerator failed");

    TEST_ASSERT_ARRAY_EQ(
        stats, single.data(), combined.data(), 2 * n, "offset advance does not match consumed counters");
}

} // namespace

// Test that consecutive calls on one generator consume the stream instead of replaying it
void test_generate_uniform_advances_offset(aclrtStream stream, OpsRandTest::TestStats& stats)
{
    TEST_CASE_BEGIN("test_generate_uniform_advances_offset");

    const uint64_t n = 64;
    const uint64_t seed = 1234; // 测试用固定 seed
    const uint64_t offset = 7;  // 测试用固定起始 offset
    float first[n];
    float second[n];
    float replay[n];

    aclrandGenerator_t gen;
    aclrandStatus_t ret = aclrandCreateGenerator(&gen, ACLRAND_RNG_PSEUDO_DEFAULT);
    TEST_ASSERT(stats, ret == ACLRAND_STATUS_SUCCESS, "aclrandCreateGenerator failed");
    ret = aclrandSetGeneratorSeed(gen, seed);
    TEST_ASSERT(stats, ret == ACLRAND_STATUS_SUCCESS, "aclrandSetGeneratorSeed failed");
    ret = aclrandSetGeneratorOffset(gen, offset);
    TEST_ASSERT(stats, ret == ACLRAND_STATUS_SUCCESS, "aclrandSetGeneratorOffset failed");

    aclrandStatus_t status = aclrandGenerateUniform(gen, first, n);
    TEST_ASSERT(stats, status == ACLRAND_STATUS_SUCCESS, "first aclrandGenerateUniform failed");
    status = aclrandGenerateUniform(gen, second, n);
    TEST_ASSERT(stats, status == ACLRAND_STATUS_SUCCESS, "second aclrandGenerateUniform failed");

    // 有状态随机流: 第二次调用必须继续消费序列, 不能重放第一段
    bool differs = false;
    for (uint64_t i = 0; i < n; ++i) {
        if (first[i] != second[i]) {
            differs = true;
            break;
        }
    }
    TEST_ASSERT(stats, differs, "consecutive calls replayed the same sequence (offset not advanced)");
    AssertOffsetAdvanceMatchesConsumption(seed, offset, n, first, second, stats);

    // 显式复位 offset 后应逐位重放第一段, 保证可复现语义不被推进破坏
    ret = aclrandSetGeneratorOffset(gen, offset);
    TEST_ASSERT(stats, ret == ACLRAND_STATUS_SUCCESS, "aclrandSetGeneratorOffset failed");
    status = aclrandGenerateUniform(gen, replay, n);
    TEST_ASSERT(stats, status == ACLRAND_STATUS_SUCCESS, "replay aclrandGenerateUniform failed");
    TEST_ASSERT_ARRAY_EQ(stats, first, replay, n, "offset reset did not replay the first sequence");

    ret = aclrandDestroyGenerator(gen);
    TEST_ASSERT(stats, ret == ACLRAND_STATUS_SUCCESS, "aclrandDestroyGenerator failed");

    TEST_CASE_PASS(stats, "test_generate_uniform_advances_offset");
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
    test_generate_uniform_advances_offset(stream, stats);
}

} // namespace GeneratorTest

// Auto-register to test framework
REGISTER_API_TEST(Generator)
