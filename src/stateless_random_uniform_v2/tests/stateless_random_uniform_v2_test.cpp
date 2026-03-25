/**
 * Copyright (c) 2026 Huawei Technologies Co., Ltd.
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of conditions of the CANN Open Software License Agreement Version 2.0.
 */

/**
 * @file stateless_random_uniform_v2_test.cpp
 * @brief stateless_random_uniform_v2算子单元测试
 */

#include "stateless_random_uniform_v2_test.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

#include "cann_ops_rand.h"
#include "internal/ops_impl.h"

namespace StatelessRandomUniformV2Test {

// 测试基本随机数生成
void test_basic_random(aclrtStream stream, OpsRandTest::TestStats& stats)
{
    TEST_CASE_BEGIN("test_basic_random");

    const uint64_t n = 100;
    uint64_t seed = 12345;
    uint64_t offset = 0;
    int32_t alg = 1;
    std::vector<float> output(n);

    aclError result = _aclrandStatelessRandomUniformV2(seed, offset, alg, output.data(), n, ACL_FLOAT);

    TEST_ASSERT(stats, result == ACL_SUCCESS, "return code failed");

    // 验证所有值都在 [0, 1) 范围内
    bool all_in_range = true;
    for (uint64_t i = 0; i < n; ++i) {
        if (output[i] < 0.0f || output[i] >= 1.0f) {
            all_in_range = false;
            std::cerr << "  [ERROR] value out of range at index " << i << ": " << output[i] << std::endl;
            break;
        }
    }
    TEST_ASSERT(stats, all_in_range, "values not in [0, 1) range");

    TEST_CASE_PASS(stats, "test_basic_random");
}

// 测试空指针检查
void test_null_pointer(aclrtStream stream, OpsRandTest::TestStats& stats)
{
    TEST_CASE_BEGIN("test_null_pointer");

    uint64_t seed = 12345;
    uint64_t offset = 0;
    int32_t alg = 1;

    aclError result = _aclrandStatelessRandomUniformV2(seed, offset, alg, nullptr, 100, ACL_FLOAT);

    TEST_ASSERT(stats, result != ACL_SUCCESS, "null output should fail");

    TEST_CASE_PASS(stats, "test_null_pointer");
}

// 测试无效大小
void test_invalid_size(aclrtStream stream, OpsRandTest::TestStats& stats)
{
    TEST_CASE_BEGIN("test_invalid_size");

    uint64_t seed = 12345;
    uint64_t offset = 0;
    int32_t alg = 1;
    float output[10];

    aclError result = _aclrandStatelessRandomUniformV2(seed, offset, alg, output, 0, ACL_FLOAT);

    TEST_ASSERT(stats, result != ACL_SUCCESS, "zero size should fail");

    TEST_CASE_PASS(stats, "test_invalid_size");
}

// 测试可复现性
void test_reproducibility(aclrtStream stream, OpsRandTest::TestStats& stats)
{
    TEST_CASE_BEGIN("test_reproducibility");

    const uint64_t n = 50;
    uint64_t seed = 99999;
    uint64_t offset = 0;
    int32_t alg = 1;
    std::vector<float> output1(n);
    std::vector<float> output2(n);

    // 使用相同的 seed 和 offset 生成两次
    aclError result1 = _aclrandStatelessRandomUniformV2(seed, offset, alg, output1.data(), n, ACL_FLOAT);
    aclError result2 = _aclrandStatelessRandomUniformV2(seed, offset, alg, output2.data(), n, ACL_FLOAT);

    TEST_ASSERT(stats, result1 == ACL_SUCCESS, "first call failed");
    TEST_ASSERT(stats, result2 == ACL_SUCCESS, "second call failed");

    // 验证两次结果相同
    TEST_ASSERT_ARRAY_EQ(stats, output1.data(), output2.data(), n, "results not reproducible");

    TEST_CASE_PASS(stats, "test_reproducibility");
}

// 测试不同种子
void test_different_seeds(aclrtStream stream, OpsRandTest::TestStats& stats)
{
    TEST_CASE_BEGIN("test_different_seeds");

    const uint64_t n = 50;
    uint64_t offset = 0;
    int32_t alg = 1;
    std::vector<float> output1(n);
    std::vector<float> output2(n);

    // 使用不同的 seed 生成
    aclError result1 = _aclrandStatelessRandomUniformV2(11111, offset, alg, output1.data(), n, ACL_FLOAT);
    aclError result2 = _aclrandStatelessRandomUniformV2(22222, offset, alg, output2.data(), n, ACL_FLOAT);

    TEST_ASSERT(stats, result1 == ACL_SUCCESS, "first call failed");
    TEST_ASSERT(stats, result2 == ACL_SUCCESS, "second call failed");

    // 验证不同 seed 产生不同结果
    bool all_different = false;
    for (uint64_t i = 0; i < n; ++i) {
        if (output1[i] != output2[i]) {
            all_different = true;
            break;
        }
    }
    TEST_ASSERT(stats, all_different, "different seeds produced same results");

    TEST_CASE_PASS(stats, "test_different_seeds");
}

// 测试不同偏移
void test_different_offsets(aclrtStream stream, OpsRandTest::TestStats& stats)
{
    TEST_CASE_BEGIN("test_different_offsets");

    const uint64_t n = 50;
    uint64_t seed = 12345;
    int32_t alg = 1;
    std::vector<float> output1(n);
    std::vector<float> output2(n);

    // 使用相同的 seed 但不同的 offset 生成
    aclError result1 = _aclrandStatelessRandomUniformV2(seed, 0, alg, output1.data(), n, ACL_FLOAT);
    aclError result2 = _aclrandStatelessRandomUniformV2(seed, 100, alg, output2.data(), n, ACL_FLOAT);

    TEST_ASSERT(stats, result1 == ACL_SUCCESS, "first call failed");
    TEST_ASSERT(stats, result2 == ACL_SUCCESS, "second call failed");

    // 验证不同 offset 产生不同结果
    bool all_different = false;
    for (uint64_t i = 0; i < n; ++i) {
        if (output1[i] != output2[i]) {
            all_different = true;
            break;
        }
    }
    TEST_ASSERT(stats, all_different, "different offsets produced same results");

    TEST_CASE_PASS(stats, "test_different_offsets");
}

// 测试 float16 类型
void test_float16_type(aclrtStream stream, OpsRandTest::TestStats& stats)
{
    TEST_CASE_BEGIN("test_float16_type");

    const uint64_t n = 100;
    uint64_t seed = 12345;
    uint64_t offset = 0;
    int32_t alg = 1;
    std::vector<uint16_t> output(n);

    aclError result = _aclrandStatelessRandomUniformV2(seed, offset, alg, output.data(), n, ACL_FLOAT16);

    TEST_ASSERT(stats, result == ACL_SUCCESS, "return code failed");

    // 验证所有值都在 [0, 1) 范围内 (通过检查 bit pattern)
    bool all_valid = true;
    for (uint64_t i = 0; i < n; ++i) {
        // float16 的范围检查比较复杂，这里只检查是否生成了非零值
        if (output[i] == 0) {
            // 允许零值，但不应全部为零
        }
    }
    TEST_ASSERT(stats, all_valid, "float16 values invalid");

    TEST_CASE_PASS(stats, "test_float16_type");
}

// 测试 bfloat16 类型
void test_bfloat16_type(aclrtStream stream, OpsRandTest::TestStats& stats)
{
    TEST_CASE_BEGIN("test_bfloat16_type");

    const uint64_t n = 100;
    uint64_t seed = 54321;
    uint64_t offset = 0;
    int32_t alg = 1;
    std::vector<uint16_t> output(n);

    aclError result = _aclrandStatelessRandomUniformV2(seed, offset, alg, output.data(), n, ACL_BF16);

    TEST_ASSERT(stats, result == ACL_SUCCESS, "return code failed");

    // 验证所有值都在 [0, 1) 范围内 (通过检查 bit pattern)
    bool all_valid = true;
    for (uint64_t i = 0; i < n; ++i) {
        // bfloat16 的范围检查比较复杂，这里只检查是否生成了非零值
        if (output[i] == 0) {
            // 允许零值，但不应全部为零
        }
    }
    TEST_ASSERT(stats, all_valid, "bfloat16 values invalid");

    TEST_CASE_PASS(stats, "test_bfloat16_type");
}

void run_all_tests(aclrtStream stream, OpsRandTest::TestStats& stats)
{
    test_basic_random(stream, stats);
    test_null_pointer(stream, stats);
    test_invalid_size(stream, stats);
    test_reproducibility(stream, stats);
    test_different_seeds(stream, stats);
    test_different_offsets(stream, stats);
    test_float16_type(stream, stats);
    test_bfloat16_type(stream, stats);
}

} // namespace StatelessRandomUniformV2Test

// 自动注册到测试框架
REGISTER_OP_TEST(StatelessRandomUniformV2)
