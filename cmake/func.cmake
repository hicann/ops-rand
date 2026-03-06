 # ----------------------------------------------------------------------------------------------------------
# Copyright (c) 2026 Huawei Technologies Co., Ltd.
# This program is free software, you can redistribute it and/or modify it under the terms and conditions of
# CANN Open Software License Agreement Version 2.0 (the "License").
# Please refer to the License for details. You may not use this file except in compliance with the License.
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
# INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
# See LICENSE in the root of the software repository for the full text of the License.
# ----------------------------------------------------------------------------------------------------------

macro(replace_cur_major_minor_ver)
    string(REPLACE CUR_MAJOR_MINOR_VER "${CANN_VERSION_${CANN_VERSION_CURRENT_PACKAGE}_VERSION_MAJOR_MINOR}" depend "${depend}")
endmacro()

# 设置包和版本号
function(set_package name)
    cmake_parse_arguments(VERSION "" "VERSION" "" ${ARGN})
    set(VERSION "${VERSION_VERSION}")
    if(NOT name)
        message(FATAL_ERROR "The name parameter is not set in set_package.")
    endif()
    if(NOT VERSION)
        message(FATAL_ERROR "The VERSION parameter is not set in set_package(${name}).")
    endif()
    string(REGEX MATCH "^([0-9]+\\.[0-9]+)" VERSION_MAJOR_MINOR "${VERSION}")
    list(APPEND CANN_VERSION_PACKAGES "${name}")
    set(CANN_VERSION_PACKAGES "${CANN_VERSION_PACKAGES}" PARENT_SCOPE)
    set(CANN_VERSION_CURRENT_PACKAGE "${name}" PARENT_SCOPE)
    set(CANN_VERSION_${name}_VERSION "${VERSION}" PARENT_SCOPE)
    set(CANN_VERSION_${name}_VERSION_MAJOR_MINOR "${VERSION_MAJOR_MINOR}" PARENT_SCOPE)
    set(CANN_VERSION_${name}_BUILD_DEPS PARENT_SCOPE)
    set(CANN_VERSION_${name}_RUN_DEPS PARENT_SCOPE)
endfunction()

# 设置构建依赖
function(set_build_dependencies pkg_name depend)
    if(NOT CANN_VERSION_CURRENT_PACKAGE)
        message(FATAL_ERROR "The set_package must be invoked first.")
    endif()
    if(NOT pkg_name)
        message(FATAL_ERROR "The pkg_name parameter is not set in set_build_dependencies.")
    endif()
    if(NOT depend)
        message(FATAL_ERROR "The depend parameter is not set in set_build_dependencies.")
    endif()
    replace_cur_major_minor_ver()
    list(APPEND CANN_VERSION_${CANN_VERSION_CURRENT_PACKAGE}_BUILD_DEPS "${pkg_name}" "${depend}")
    set(CANN_VERSION_${CANN_VERSION_CURRENT_PACKAGE}_BUILD_DEPS "${CANN_VERSION_${CANN_VERSION_CURRENT_PACKAGE}_BUILD_DEPS}" PARENT_SCOPE)
endfunction()

# 设置运行依赖
function(set_run_dependencies pkg_name depend)
    if(NOT CANN_VERSION_CURRENT_PACKAGE)
        message(FATAL_ERROR "The set_package must be invoked first.")
    endif()
    if(NOT pkg_name)
        message(FATAL_ERROR "The pkg_name parameter is not set in set_run_dependencies.")
    endif()
    if(NOT depend)
        message(FATAL_ERROR "The depend parameter is not set in set_run_dependencies.")
    endif()
    replace_cur_major_minor_ver()
    list(APPEND CANN_VERSION_${CANN_VERSION_CURRENT_PACKAGE}_RUN_DEPS "${pkg_name}" "${depend}")
    set(CANN_VERSION_${CANN_VERSION_CURRENT_PACKAGE}_RUN_DEPS "${CANN_VERSION_${CANN_VERSION_CURRENT_PACKAGE}_RUN_DEPS}" PARENT_SCOPE)
endfunction()

# 检查构建依赖
function(check_pkg_build_deps pkg_name)
    execute_process(
        COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/scripts/check_build_dependencies.py "$ENV{ASCEND_HOME_PATH}" ${CANN_VERSION_${pkg_name}_BUILD_DEPS}
        RESULT_VARIABLE result
    )
    if(result)
        message(FATAL_ERROR "Check ${pkg_name} build dependencies failed!")
    endif()
endfunction()

# 添加生成version.info的目标
# 目标名格式为：version_${包名}_info
function(add_version_info_targets)
  foreach(pkg_name ${CANN_VERSION_PACKAGES})
    add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/version.${pkg_name}.info
      COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/scripts/generate_version_info.py --output ${CMAKE_BINARY_DIR}/version.${pkg_name}.info
              "${CANN_VERSION_${pkg_name}_VERSION}" ${CANN_VERSION_${pkg_name}_RUN_DEPS}
      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/version.cmake ${CMAKE_CURRENT_SOURCE_DIR}/scripts/generate_version_info.py
      VERBATIM
    )
    add_custom_target(version_${pkg_name}_info ALL DEPENDS ${CMAKE_BINARY_DIR}/version.${pkg_name}.info)
  endforeach()
endfunction()

function(register_operator)
    # 解析参数
    cmake_parse_arguments(ARG
        ""                              # 选项
        "NAME;ARCH_DIR"                 # 单值参数
        "SOURCES;HEADERS"               # 多值参数
        ${ARGN}
    )
 
    # 必需参数检查
    if(NOT ARG_NAME)
        message(FATAL_ERROR "register_operator: NAME parameter is required")
    endif()
 
    if(NOT ARG_SOURCES)
        message(FATAL_ERROR "register_operator: SOURCES parameter is required for operator '${ARG_NAME}'")
    endif()
 
    # 设置默认架构目录
    if(NOT ARG_ARCH_DIR)
        set(ARG_ARCH_DIR "arch35")
    endif()
 
    # 算子名称转大写
    string(TOUPPER ${ARG_NAME} OP_UPPER)
 
    # 算子源文件路径
    set(OP_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
    message(STATUS "Found op_source_dir is ${OP_SOURCE_DIR}/")
    
    # 查找架构特定源文件
    if(EXISTS ${OP_SOURCE_DIR}/${ARG_ARCH_DIR})
        # 检查架构目录中的 .cpp 文件
        file(GLOB ARCH_SOURCES ${OP_SOURCE_DIR}/${ARG_ARCH_DIR}/*.cpp)
 
        if(ARCH_SOURCES)
            list(APPEND ARG_SOURCES ${ARCH_SOURCES})
            message(STATUS "    Found architecture-specific sources in ${ARG_ARCH_DIR}/")
        endif()
    endif()
 
    # 将相对源文件转换为绝对路径
    set(absolute_sources "")
    foreach(src ${ARG_SOURCES})
        if(IS_ABSOLUTE ${src})
            set(abs_src ${src})
        else()
            set(abs_src ${OP_SOURCE_DIR}/${src})
        endif()
        list(APPEND absolute_sources ${abs_src})
        
        # ========= 关键修改：直接添加源码到库（无需全局变量传递）=========
        target_sources(${OPS_RAND} PRIVATE ${abs_src})
    endforeach()

    # ========= 头文件包含目录 =========
    target_include_directories(${OPS_RAND} PRIVATE
        ${OP_SOURCE_DIR}
        ${OP_SOURCE_DIR}/${ARG_ARCH_DIR}
    )

    # ========= 编译宏定义 =========
    target_compile_definitions(${OPS_RAND} PRIVATE
        -DASCENDC_ASSERT=
        ENABLE_OPERATOR_${OP_UPPER}=1
    )

    # ========= 输出注册信息 =========
    message(STATUS "  Registered operator: ${ARG_NAME}")
    message(STATUS "    Sources: ${ARG_SOURCES}")
endfunction()

function(register_operator_test OP_NAME)
    # 1. 收集测试源码（当前test目录下所有cpp）
    file(GLOB TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
    if(TEST_SOURCES STREQUAL "")
        message(WARNING "  No test sources found for operator ${OP_NAME}")
        return()
    endif()

    # 2. 测试可执行文件名称（固定格式：test_${算子名}_example）
    set(TEST_EXEC_NAME "test_${OP_NAME}_example")

    # 3. 创建测试可执行文件
    add_executable(${TEST_EXEC_NAME} ${TEST_SOURCES})

    # 4. 链接算子库 + ACL核心库 + 运行时库 + 系统库（仅链接存在的库）
    target_link_libraries(${TEST_EXEC_NAME}
        PRIVATE
        ${OPS_RAND}  # 算子库
        ${ASCEND_HOME_PATH}/lib64/libascendcl.so
        ${ASCEND_HOME_PATH}/lib64/libacl_rt.so
    )

    # 5. 显式依赖，保证库先编译
    add_dependencies(${TEST_EXEC_NAME} ${OPS_RAND})

    # 6. 包含目录
    target_include_directories(${TEST_EXEC_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${ASCEND_HOME_PATH}/include
    )

    # 7. 编译选项
    target_compile_options(${TEST_EXEC_NAME} PRIVATE
        -std=c++14
        -g
        -Wall
        -fPIC
    )

    # 8. 设置运行时库路径（包含ACL库路径）
    set_target_properties(${TEST_EXEC_NAME} PROPERTIES
        BUILD_RPATH "$<TARGET_FILE_DIR:${OPS_RAND}>:${ASCEND_HOME_PATH}/lib64"
        INSTALL_RPATH "$ORIGIN/../lib:${ASCEND_HOME_PATH}/lib64"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests"
    )

    # 9. 注册到CTest
    add_test(
        NAME ${TEST_EXEC_NAME}
        COMMAND ${CMAKE_BINARY_DIR}/tests/${TEST_EXEC_NAME}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests
    )

    # 10. 设置测试属性（超时、标签）
    set_tests_properties(${TEST_EXEC_NAME} PROPERTIES
        LABELS "operator;${OP_NAME}"
        TIMEOUT 300
    )

    # 输出日志
    message(STATUS "  Configured test for operator: ${OP_NAME} (exec: ${TEST_EXEC_NAME})")
endfunction()