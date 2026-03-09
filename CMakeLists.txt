 # ----------------------------------------------------------------------------------------------------------
# Copyright (c) 2026 Huawei Technologies Co., Ltd.
# This program is free software, you can redistribute it and/or modify it under the terms and conditions of
# CANN Open Software License Agreement Version 2.0 (the "License").
# Please refer to the License for details. You may not use this file except in compliance with the License.
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
# INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
# See LICENSE in the root of the software repository for the full text of the License.
# ----------------------------------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.16.0)

# ========= 编译配置 =========
set(OPS_RAND cann_ops_rand)

find_package(ASC REQUIRED)

project(${OPS_RAND} VERSION 1.0.0 LANGUAGES ASC CXX)

# ========= 基本配置 =========
set(SOC_VERSION "" CACHE STRING "Target SoC version (ascend950, ascend910b, etc.)")
set(ASCEND_CANN_PACKAGE_PATH $ENV{ASCEND_HOME_PATH})
set(RUN_MODE "npu" CACHE STRING "Run mode: npu")
set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type (Release/Debug)")

# 测试相关配置（仅BUILD_TEST=ON时生效）
set(BUILD_TEST OFF CACHE BOOL "Whether to build tests")
set(COMPILED_OPS "" CACHE STRING "List of operators to test (comma-separated)")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests)

# 打包相关选项
option(ENABLE_PACKAGE "Enable package build" OFF)
option(ENABLE_BINARY "Build binary kernel" ON)
option(ENABLE_TEST "Enable unit tests" OFF)

# 设置第三方库路径
if(NOT CANN_3RD_LIB_PATH)
    set(CANN_3RD_LIB_PATH ${PROJECT_SOURCE_DIR}/third_party CACHE STRING "CANN third party lib path")
endif()

# ========= 包含配置文件 =========
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# 包含函数定义
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/func.cmake")
    include(cmake/func.cmake)
endif()

# 包含版本配置
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/version.cmake")
    include(version.cmake)
endif()

# 检查构建依赖（可选，当BUILD_WITH_INSTALLED_DEPENDENCY_CANN_PKG时执行）
if(BUILD_WITH_INSTALLED_DEPENDENCY_CANN_PKG AND COMMAND check_pkg_build_deps)
    check_pkg_build_deps("ops_rand")
endif()

# 生成版本信息文件
if(COMMAND add_version_info_targets)
    add_version_info_targets()
endif()

# 添加 CANN 的 cmake 模块路径（用于 FindASC.cmake）
if(UNIX)
    set(SYSTEM_PREFIX "${CMAKE_SYSTEM_PROCESSOR}-linux")
endif()
if(DEFINED ENV{ASCEND_HOME_PATH})
    list(APPEND CMAKE_MODULE_PATH "$ENV{ASCEND_HOME_PATH}/${SYSTEM_PREFIX}/tikcpp/ascendc_kernel_cmake/asc_modules")
elseif(DEFINED ASCEND_HOME_PATH)
    list(APPEND CMAKE_MODULE_PATH "${ASCEND_HOME_PATH}/${SYSTEM_PREFIX}/tikcpp/ascendc_kernel_cmake/asc_modules")
endif()

# 检查 cmake 配置文件是否存在
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/variables.cmake")
    include(cmake/variables.cmake)
endif()

# ========= 提前创建库 =========
add_library(${OPS_RAND} SHARED)	 

set_target_properties(${OPS_RAND} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

if(BUILD_TEST)
    # 启用CTest
    enable_testing()
endif()

# ========= 进入 src 目录 =========
add_subdirectory(src)	 

# ========= 检查库是否有源码 =========
get_target_property(LIB_SOURCES ${OPS_RAND} SOURCES)
if(NOT LIB_SOURCES OR LIB_SOURCES STREQUAL "")
    message(FATAL_ERROR "No sources added to target")
endif()

# ========= 所有文件标记为 ASC =========
set_source_files_properties( 
    ${LIB_SOURCES} 
    PROPERTIES LANGUAGE ASC 
)

target_include_directories(${OPS_RAND} PRIVATE
    $ENV{ASCEND_HOME_PATH}/pkg_inc/op_common/
    $ENV{ASCEND_HOME_PATH}/pkg_inc/base/
    $ENV{ASCEND_HOME_PATH}/pkg_inc/
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${OPS_RAND} PRIVATE
    tiling_api
)

# 使用 SoC 特定的编译选项（如果定义了）
if(DEFINED SOC_COMPILE_OPTIONS)
    target_compile_options(${OPS_RAND} PRIVATE
        $<$<COMPILE_LANGUAGE:ASC>:${SOC_COMPILE_OPTIONS}>
    )
elseif(SOC_VERSION STREQUAL "ascend950" OR SOC_VERSION STREQUAL "ascend950dt_9595")
    target_compile_options(${OPS_RAND} PRIVATE
        $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-3510>
    )
endif()

# ========= 安装规则 =========
# 安装库文件
install(TARGETS ${OPS_RAND}
    LIBRARY DESTINATION ${OPS_RAND_LIB_INSTALL_DIR}
)

# 安装头文件
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION ${OPS_RAND_INC_INSTALL_DIR}
    FILES_MATCHING PATTERN "*.h"
)

# ========= 打包配置 =========
if(ENABLE_PACKAGE)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/package.cmake")
        include(cmake/package.cmake)
        if(COMMAND pack_built_in)
            pack_built_in()
        endif()
    else()
        message(WARNING "Package configuration file not found: cmake/package.cmake")
    endif()
endif()