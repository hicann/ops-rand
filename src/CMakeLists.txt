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

# 引入注册函数
include(${CMAKE_SOURCE_DIR}/cmake/func.cmake)

# 查找src下的所有算子目录
file(GLOB OP_DIRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*)

# 过滤有效算子目录（排除文件、common）
set(VALID_OP_DIRS "")
foreach(DIR ${OP_DIRS})
    if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${DIR} AND NOT ${DIR} STREQUAL "common")
        list(APPEND VALID_OP_DIRS ${DIR})
    endif()
endforeach()

# 传递有效算子列表到根目录
set(VALID_OP_DIRS ${VALID_OP_DIRS} PARENT_SCOPE)

# 检查有效算子目录
if(VALID_OP_DIRS STREQUAL "")
    message(WARNING "No valid operator directories found in src/")
else()
    message(STATUS "Found valid operator directories: ${VALID_OP_DIRS}")
endif()

# 收集common目录源码
if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/common)
    file(GLOB COMMON_SRC ${CMAKE_CURRENT_SOURCE_DIR}/common/*.cpp)
    if(COMMON_SRC)
        target_sources(${OPS_RAND} PRIVATE ${COMMON_SRC})
        message(STATUS "Collected common sources: ${COMMON_SRC}")
    endif()
endif()

# 预处理COMPILED_OPS为算子列表
set(TARGET_OP_LIST "")
if(DEFINED COMPILED_OPS AND NOT COMPILED_OPS STREQUAL "")
    if(COMPILED_OPS STREQUAL "all")
        # 如果是"all"，编译所有算子
        set(TARGET_OP_LIST ${VALID_OP_DIRS})
        message(STATUS "COMPILED_OPS=all - building all operators: ${TARGET_OP_LIST}")
    else()
        # 否则按逗号分割指定的算子列表
        string(REPLACE "," ";" TARGET_OP_LIST ${COMPILED_OPS})
        message(STATUS "Specified operators to build: ${TARGET_OP_LIST}")
        
        # 检查指定算子是否存在
        foreach(TARGET_OP ${TARGET_OP_LIST})
            # 去除可能的空格
            string(STRIP ${TARGET_OP} TARGET_OP)
            if(NOT "${TARGET_OP}" IN_LIST VALID_OP_DIRS)
                message(FATAL_ERROR "Specified operator '${TARGET_OP}' does not exist. Valid operators: ${VALID_OP_DIRS}")
            endif()
        endforeach()
    endif()
else()
    # 如果没有指定COMPILED_OPS，默认编译所有算子
    set(TARGET_OP_LIST ${VALID_OP_DIRS})
    message(STATUS "COMPILED_OPS not specified, building all: ${TARGET_OP_LIST}")
endif()

# 遍历所有有效算子目录，只编译指定的算子
foreach(OP_DIR ${VALID_OP_DIRS})
    # 使用list(FIND)检查当前算子是否在目标列表中
    list(FIND TARGET_OP_LIST ${OP_DIR} OP_INDEX)
    if(OP_INDEX GREATER -1)  # 找到了
        # 编译算子核心代码
        if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${OP_DIR}/CMakeLists.txt)
            add_subdirectory(${OP_DIR})
            message(STATUS "Processed operator directory: ${OP_DIR}")
            
            # 仅BUILD_TEST=ON时处理测试目录
            if(BUILD_TEST)
                set(TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${OP_DIR}/tests)
                if(IS_DIRECTORY ${TEST_DIR})
                    message(STATUS "Processing test directory for operator: ${OP_DIR}")
                    add_subdirectory(${TEST_DIR})
                else()
                    message(STATUS "No tests directory found for operator: ${OP_DIR}")
                endif()
            endif()
        else()
            message(WARNING "Operator directory ${OP_DIR} missing CMakeLists.txt, skipped")
        endif()
    else()
        message(STATUS "Skipping non-target operator directory: ${OP_DIR}")
    endif()
endforeach()

# 输出库源码
get_target_property(ALL_SRC ${OPS_RAND} SOURCES)
message(STATUS "Sources added to cannops_rand library: ${ALL_SRC}")