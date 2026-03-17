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
 * @brief Generator API 单元测试
 */

#include "generator_test.h"
#include "cann_ops_rand.h"
#include <iostream>

// 测试创建 generator
void test_create_generator(aclrtStream stream, OpsRandTest::TestStats& stats)
{
    TEST_CASE_BEGIN("test_create_generator");

    aclrandGenerator_t generator;
    aclError ret = aclrandCreateGenerator(&generator, ACLRAND_RNG_PSEUDO_DEFAULT);
    TEST_ASSERT(stats, ret == ACL_SUCCESS, "aclrandCreateGenerator failed");
    TEST_ASSERT(stats, generator != nullptr, "generator should not be null");

    ret = aclrandDestroyGenerator(generator);
    TEST_ASSERT(stats, ret == ACL_SUCCESS, "aclrandDestroyGenerator failed");

    TEST_CASE_PASS(stats, "test_create_generator");
}

// 测试空指针创建
void test_create_null_generator(aclrtStream stream, OpsRandTest::TestStats& stats)
{
    TEST_CASE_BEGIN("test_create_null_generator");

    aclError ret = aclrandCreateGenerator(nullptr, ACLRAND_RNG_PSEUDO_DEFAULT);
    TEST_ASSERT(stats, ret == ACL_ERROR, "aclrandCreateGenerator with null should fail");

    TEST_CASE_PASS(stats, "test_create_null_generator");
}

// 测试设置 seed
void test_set_seed(aclrtStream stream, OpsRandTest::TestStats& stats)
{
    TEST_CASE_BEGIN("test_set_seed");

    aclrandGenerator_t generator;
    aclError ret = aclrandCreateGenerator(&generator, ACLRAND_RNG_PSEUDO_DEFAULT);
    TEST_ASSERT(stats, ret == ACL_SUCCESS, "aclrandCreateGenerator failed");

    ret = aclrandSetGeneratorSeed(generator, 12345);
    TEST_ASSERT(stats, ret == ACL_SUCCESS, "aclrandSetGeneratorSeed failed");

    ret = aclrandSetGeneratorSeed(nullptr, 12345);
    TEST_ASSERT(stats, ret == ACL_ERROR, "aclrandSetGeneratorSeed with null should fail");

    ret = aclrandDestroyGenerator(generator);
    TEST_ASSERT(stats, ret == ACL_SUCCESS, "aclrandDestroyGenerator failed");

    TEST_CASE_PASS(stats, "test_set_seed");
}

// 测试设置 offset
void test_set_offset(aclrtStream stream, OpsRandTest::TestStats& stats)
{
    TEST_CASE_BEGIN("test_set_offset");

    aclrandGenerator_t generator;
    aclError ret = aclrandCreateGenerator(&generator, ACLRAND_RNG_PSEUDO_DEFAULT);
    TEST_ASSERT(stats, ret == ACL_SUCCESS, "aclrandCreateGenerator failed");

    ret = aclrandSetGeneratorOffset(generator, 100);
    TEST_ASSERT(stats, ret == ACL_SUCCESS, "aclrandSetGeneratorOffset failed");

    ret = aclrandSetGeneratorOffset(nullptr, 100);
    TEST_ASSERT(stats, ret == ACL_ERROR, "aclrandSetGeneratorOffset with null should fail");

    ret = aclrandDestroyGenerator(generator);
    TEST_ASSERT(stats, ret == ACL_SUCCESS, "aclrandDestroyGenerator failed");

    TEST_CASE_PASS(stats, "test_set_offset");
}

// 测试设置 stream
void test_set_stream(aclrtStream stream, OpsRandTest::TestStats& stats)
{
    TEST_CASE_BEGIN("test_set_stream");

    aclrandGenerator_t generator;
    aclError ret = aclrandCreateGenerator(&generator, ACLRAND_RNG_PSEUDO_DEFAULT);
    TEST_ASSERT(stats, ret == ACL_SUCCESS, "aclrandCreateGenerator failed");

    ret = aclrandSetGeneratorStream(generator, stream);
    TEST_ASSERT(stats, ret == ACL_SUCCESS, "aclrandSetGeneratorStream failed");

    ret = aclrandSetGeneratorStream(nullptr, stream);
    TEST_ASSERT(stats, ret == ACL_ERROR, "aclrandSetGeneratorStream with null generator should fail");

    ret = aclrandDestroyGenerator(generator);
    TEST_ASSERT(stats, ret == ACL_SUCCESS, "aclrandDestroyGenerator failed");

    TEST_CASE_PASS(stats, "test_set_stream");
}

// 测试销毁空 generator
void test_destroy_null_generator(aclrtStream stream, OpsRandTest::TestStats& stats)
{
    TEST_CASE_BEGIN("test_destroy_null_generator");

    aclError ret = aclrandDestroyGenerator(nullptr);
    TEST_ASSERT(stats, ret == ACL_ERROR, "aclrandDestroyGenerator with null should fail");

    TEST_CASE_PASS(stats, "test_destroy_null_generator");
}

// 测试 aclrandGenerateUniform 基本功能
void test_generate_uniform(aclrtStream stream, OpsRandTest::TestStats& stats)
{
    TEST_CASE_BEGIN("test_generate_uniform");

    // 创建 generator
    aclrandGenerator_t generator;
    aclError ret = aclrandCreateGenerator(&generator, ACLRAND_RNG_PSEUDO_DEFAULT);
    TEST_ASSERT(stats, ret == ACL_SUCCESS, "aclrandCreateGenerator failed");

    // 设置参数
    ret = aclrandSetGeneratorSeed(generator, 12345);
    TEST_ASSERT(stats, ret == ACL_SUCCESS, "aclrandSetGeneratorSeed failed");

    ret = aclrandSetGeneratorOffset(generator, 0);
    TEST_ASSERT(stats, ret == ACL_SUCCESS, "aclrandSetGeneratorOffset failed");

    // 生成随机数
    const uint64_t n = 100;
    float output[n];
    ret = aclrandGenerateUniform(&generator, output, n);
    TEST_ASSERT(stats, ret == ACL_SUCCESS, "aclrandGenerateUniform failed");

    // 验证所有值都在 [0, 1) 范围内
    bool all_in_range = true;
    for (uint64_t i = 0; i < n; ++i) {
        if (output[i] < 0.0f || output[i] >= 1.0f) {
            all_in_range = false;
            break;
        }
    }
    TEST_ASSERT(stats, all_in_range, "values not in [0, 1) range");

    ret = aclrandDestroyGenerator(generator);
    TEST_ASSERT(stats, ret == ACL_SUCCESS, "aclrandDestroyGenerator failed");

    TEST_CASE_PASS(stats, "test_generate_uniform");
}

// 测试 aclrandGenerateUniform 空指针参数
void test_generate_uniform_null(aclrtStream stream, OpsRandTest::TestStats& stats)
{
    TEST_CASE_BEGIN("test_generate_uniform_null");

    aclrandGenerator_t generator;
    aclError ret = aclrandCreateGenerator(&generator, ACLRAND_RNG_PSEUDO_DEFAULT);
    TEST_ASSERT(stats, ret == ACL_SUCCESS, "aclrandCreateGenerator failed");

    float output[10];

    // 空 generator 指针
    ret = aclrandGenerateUniform(nullptr, output, 10);
    TEST_ASSERT(stats, ret == ACL_ERROR, "aclrandGenerateUniform with null generator ptr should fail");

    // 空 output
    ret = aclrandGenerateUniform(&generator, nullptr, 10);
    TEST_ASSERT(stats, ret == ACL_ERROR, "aclrandGenerateUniform with null output should fail");

    // 零大小
    ret = aclrandGenerateUniform(&generator, output, 0);
    TEST_ASSERT(stats, ret == ACL_ERROR, "aclrandGenerateUniform with zero n should fail");

    ret = aclrandDestroyGenerator(generator);
    TEST_ASSERT(stats, ret == ACL_SUCCESS, "aclrandDestroyGenerator failed");

    TEST_CASE_PASS(stats, "test_generate_uniform_null");
}

// 测试 aclrandGenerateUniform 可复现性
void test_generate_uniform_reproducibility(aclrtStream stream, OpsRandTest::TestStats& stats)
{
    TEST_CASE_BEGIN("test_generate_uniform_reproducibility");

    const uint64_t n = 50;
    float output1[n];
    float output2[n];

    // 第一次生成
    aclrandGenerator_t gen1;
    aclError ret = aclrandCreateGenerator(&gen1, ACLRAND_RNG_PSEUDO_DEFAULT);
    TEST_ASSERT(stats, ret == ACL_SUCCESS, "aclrandCreateGenerator failed");
    ret = aclrandSetGeneratorSeed(gen1, 99999);
    TEST_ASSERT(stats, ret == ACL_SUCCESS, "aclrandSetGeneratorSeed failed");
    ret = aclrandSetGeneratorOffset(gen1, 0);
    TEST_ASSERT(stats, ret == ACL_SUCCESS, "aclrandSetGeneratorOffset failed");
    ret = aclrandGenerateUniform(&gen1, output1, n);
    TEST_ASSERT(stats, ret == ACL_SUCCESS, "aclrandGenerateUniform failed");
    ret = aclrandDestroyGenerator(gen1);
    TEST_ASSERT(stats, ret == ACL_SUCCESS, "aclrandDestroyGenerator failed");

    // 第二次生成（相同参数）
    aclrandGenerator_t gen2;
    ret = aclrandCreateGenerator(&gen2, ACLRAND_RNG_PSEUDO_DEFAULT);
    TEST_ASSERT(stats, ret == ACL_SUCCESS, "aclrandCreateGenerator failed");
    ret = aclrandSetGeneratorSeed(gen2, 99999);
    TEST_ASSERT(stats, ret == ACL_SUCCESS, "aclrandSetGeneratorSeed failed");
    ret = aclrandSetGeneratorOffset(gen2, 0);
    TEST_ASSERT(stats, ret == ACL_SUCCESS, "aclrandSetGeneratorOffset failed");
    ret = aclrandGenerateUniform(&gen2, output2, n);
    TEST_ASSERT(stats, ret == ACL_SUCCESS, "aclrandGenerateUniform failed");
    ret = aclrandDestroyGenerator(gen2);
    TEST_ASSERT(stats, ret == ACL_SUCCESS, "aclrandDestroyGenerator failed");

    // 验证两次结果相同
    TEST_ASSERT_ARRAY_EQ(stats, output1, output2, n, "results not reproducible");

    TEST_CASE_PASS(stats, "test_generate_uniform_reproducibility");
}

// 导出函数
namespace GeneratorTest {
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
}
} // namespace GeneratorTest

// 自动注册到测试框架
REGISTER_API_TEST(Generator)
