 # ----------------------------------------------------------------------------------------------------------
# Copyright (c) 2026 Huawei Technologies Co., Ltd.
# This program is free software, you can redistribute it and/or modify it under the terms and conditions of
# CANN Open Software License Agreement Version 2.0 (the "License").
# Please refer to the License for details. You may not use this file except in compliance with the License.
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
# INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
# See LICENSE in the root of the software repository for the full text of the License.
# ----------------------------------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.16)

##############################################################################
# 创建 ops_rand 动态库
##############################################################################

add_library(${OPS_RAND} SHARED)

# 设置库的版本属性
set_target_properties(${OPS_RAND} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    OUTPUT_NAME "cann_ops_rand"  # 生成文件名：libcann_ops_rand.so
)

# 设置公共包含目录
target_include_directories(${OPS_RAND} PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${ASCEND_HOME_PATH}/include               # acl.h 等核心头文件
    ${ASCEND_HOME_PATH}/pkg_inc/op_common/    # ASCEND 公共操作（kernel_operator.h 等）
    ${ASCEND_HOME_PATH}/pkg_inc/base/         # ASCEND 基础类型
    ${ASCEND_HOME_PATH}/pkg_inc/              # ASCEND 头文件
)

# 设置编译选项
target_compile_options(${OPS_RAND} PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${ASCEND_NPU_ARCH}>
)

# 链接 CANN tiling 平台库 (提供 GetCoreNumAiv 等函数)
target_link_libraries(${OPS_RAND} PRIVATE
    tiling_api
)

##############################################################################
# 添加各算子子目录
##############################################################################

message(STATUS "==========================================")
message(STATUS "Registering operators...")
message(STATUS "==========================================")

# 收集 api 目录源码
if(IS_DIRECTORY ${CMAKE_SOURCE_DIR}/api)
    file(GLOB API_SRC ${CMAKE_SOURCE_DIR}/api/*.cpp)
    if(API_SRC)
        target_sources(${OPS_RAND} PRIVATE ${API_SRC})
        message(STATUS "Collected api sources: ${API_SRC}")
    endif()
endif()

foreach(OP ${BUILD_OPERATORS})
    if(EXISTS ${CMAKE_SOURCE_DIR}/src/${OP}/CMakeLists.txt)
        add_subdirectory(${OP})
        message(STATUS "Added operator directory: ${OP}")
    else()
        message(WARNING "Operator ${OP} missing CMakeLists.txt, skipping")
    endif()
endforeach()

get_target_property(LIB_TARGET_SOURCE ${OPS_RAND} SOURCES)
if (NOT LIB_TARGET_SOURCE OR LIB_TARGET_SOURCE STREQUAL "")
    message(FATAL_ERROR "No sources added to cann_ops_rand target")
endif()

set_source_files_properties(
    ${LIB_TARGET_SOURCE}
    PROPERTIES LANGUAGE ASC
)

message(STATUS "==========================================")
message(STATUS "Operator registration completed")
message(STATUS "==========================================")
message(STATUS "")

message(STATUS "Operator library 'ops_rand' configured successfully")
