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
#include "cann_ops_rand.h"
#include <new>




int aclrandCreateGenerator(aclrandGenerator_t *generator, aclrandRngType_t rng_type) {
	if (generator == nullptr) {
		return ACL_ERROR;
	}
	opsrand::Generator *g = nullptr;
	try {
		g = new opsrand::Generator();
	} catch (const std::bad_alloc &) {
		return ACL_ERROR;
	}
	g->seed = 0;
	g->rng_type = static_cast<int>(rng_type);
	g->offset = 0;
	g->stream_id = 0;
	*generator = reinterpret_cast<aclrandGenerator_t>(g);
	return ACL_SUCCESS;
}

int aclrandSetGeneratorSeed(aclrandGenerator_t generator, uint64_t seed) {
	if (generator == nullptr) {
		return ACL_ERROR;
	}
	opsrand::Generator *g = reinterpret_cast<opsrand::Generator*>(generator);
	g->seed = seed;
	return ACL_SUCCESS;
}

int aclrandSetGeneratorOffset(aclrandGenerator_t generator, size_t offset) {
	if (generator == nullptr) {
		return ACL_ERROR;
	}
	opsrand::Generator *g = reinterpret_cast<opsrand::Generator*>(generator);
	g->offset = offset;
	return ACL_SUCCESS;
}

int aclrandSetGeneratorStream(aclrandGenerator_t generator, aclrtStream stream_id) {
	if (generator == nullptr) {
		return ACL_ERROR;
	}
	opsrand::Generator *g = reinterpret_cast<opsrand::Generator*>(generator);
	g->stream_id = stream_id;
	return ACL_SUCCESS;
}

int aclrandDestroyGenerator(aclrandGenerator_t generator) {
	if (generator == nullptr) {
		return ACL_ERROR;
	}
	opsrand::Generator *g = reinterpret_cast<opsrand::Generator*>(generator);
	delete g;
	return ACL_SUCCESS;
}

int aclrandGenerateUniform(aclrandGenerator_t *generator, float *output, uint64_t n) {
	if (generator == nullptr || *generator == nullptr || output == nullptr || n == 0) {
		return ACL_ERROR;
	}

	opsrand::Generator *g = reinterpret_cast<opsrand::Generator*>(*generator);

	return aclrandStatelessRandomUniformV2(
		g->seed,
		g->offset,
		1,  // alg
		output,
		n,
		ACL_FLOAT
	);
}
