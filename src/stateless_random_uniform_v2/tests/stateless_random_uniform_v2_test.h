/**
 * Copyright (c) 2026 Huawei Technologies Co., Ltd.
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of conditions of the CANN Open Software License Agreement Version 2.0.
 */

/**
 * @file stateless_random_uniform_v2_test.h
 * @brief stateless_random_uniform_v2算子测试函数声明
 */

#pragma once

#include "test_common.h"

namespace StatelessRandomUniformV2Test {
    void run_all_tests(aclrtStream stream, OpsRandTest::TestStats& stats);
}
