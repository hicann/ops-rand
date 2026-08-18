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
# CANN_INC_ROOT 由 cmake/init_env.cmake 自动探测（兼容旧版 pkg_inc 与新版 <arch>-linux/include 布局）
target_include_directories(${OPS_RAND} PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${ASCEND_HOME_PATH}/include               # acl.h 等核心头文件
    ${CANN_INC_ROOT}/op_common/               # ASCEND 公共操作（kernel_operator.h、op_kernel 等）
    ${CANN_INC_ROOT}/base/                    # ASCEND 基础类型
    ${CANN_INC_ROOT}/                         # ASCEND 头文件
)

# 设置编译选项
target_compile_options(${OPS_RAND} PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${ASCEND_NPU_ARCH}>
)

# 链接 CANN tiling 平台库与 Ascend C 宿主侧运行时
# host 侧源码保持 CXX 后，目标链接语言为 CXX，需显式指定库搜索路径并补齐
# 运行时依赖（此前全 ASC 编译时由 bisheng/ccec 驱动链接自动携带）：
#   - tiling_api      提供 GetCoreNumAiv 等 tiling 平台函数（devlib）
#   - ascendc_runtime 提供 AscendLaunchKernelWithHostArgs 等 kernel 宿主侧符号（lib64）
# 路径由 cmake/init_env.cmake 探测（CANN_DEVLIB_ROOT / CANN_LIB64_ROOT）
target_link_directories(${OPS_RAND} PRIVATE
    ${CANN_DEVLIB_ROOT}
    ${CANN_LIB64_ROOT}
)
target_link_libraries(${OPS_RAND} PRIVATE
    tiling_api
    ascendc_runtime
)

##############################################################################
# 添加各算子子目录
##############################################################################

message(STATUS "==========================================")
message(STATUS "Registering operators...")
message(STATUS "==========================================")

# 收集 lib 目录源码
# lib/ 下为 host 侧普通 C++ 实现（调用 ACL API），保持默认 CXX 语言，
# 不标记为 ASC；显式列出源文件，避免 file(GLOB) 新增文件后不触发重新 configure
if(EXISTS ${CMAKE_SOURCE_DIR}/lib/generator.cpp)
    target_sources(${OPS_RAND} PRIVATE
        ${CMAKE_SOURCE_DIR}/lib/generator.cpp
    )
    message(STATUS "Collected lib sources: ${CMAKE_SOURCE_DIR}/lib/generator.cpp")
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

# 仅对 register_operator() 注册的 kernel 源文件标记 ASC 语言（由 ccec 编译，
# 使用 --npu-arch 等选项）；lib/ 下的 host 侧普通 C++ 保持默认 CXX。
# 注意：必须在目标所在目录作用域设置源文件属性，子目录中设置不可见。
get_property(OPS_RAND_KERNEL_SOURCES GLOBAL PROPERTY OPS_RAND_KERNEL_SOURCES)
if(NOT OPS_RAND_KERNEL_SOURCES)
    message(FATAL_ERROR "No kernel sources registered by register_operator()")
endif()
set_source_files_properties(
    ${OPS_RAND_KERNEL_SOURCES}
    PROPERTIES LANGUAGE ASC
)

message(STATUS "==========================================")
message(STATUS "Operator registration completed")
message(STATUS "==========================================")
message(STATUS "")

message(STATUS "Operator library 'ops_rand' configured successfully")
