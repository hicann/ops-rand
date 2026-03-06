#!/bin/bash
# ----------------------------------------------------------------------------------------------------------
# Copyright (c) 2026 Huawei Technologies Co., Ltd.
# This program is free software, you can redistribute it and/or modify it under the terms and conditions of
# CANN Open Software License Agreement Version 2.0 (the "License").
# Please refer to the License for details. You may not use this file except in compliance with the License.
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
# INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
# See LICENSE in the root of the software repository for the full text of the License.
# ----------------------------------------------------------------------------------------------------------

set -e

BUILD_DIR=build
BUILD_OUT_DIR=build_out
BUILD_OP=""
RUN_TEST=OFF
ENABLE_PACKAGE=OFF
SOC_VERSION="ascend950"
THREAD_NUM=$(nproc)
BUILD_TYPE=Release
VERBOSE=""

# 获取脚本所在目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CANN_3RD_LIB_PATH="${SCRIPT_DIR}/third_party"
SUPPORTED_SOCS=("ascend950")

# 日志函数
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

log_verbose() {
    if [ "$VERBOSE" = true ]; then
        echo -e "${BLUE}[VERBOSE]${NC} $1"
    fi
}

show_supported_socs() {
    log_info "Supported SoC versions:"
    for soc in "${SUPPORTED_SOCS[@]}"; do
        echo "  - ${soc}"
    done
}

validate_soc_version() {
    local soc=$1
    for supported_soc in "${SUPPORTED_SOCS[@]}"; do
        if [ "$soc" = "$supported_soc" ]; then
            return 0  # 支持
        fi
    done
    return 1  # 不支持
}

# ==========================
# 帮助信息
# ==========================
show_help() {
    cat << EOF
ops-rand Build Script

Usage: bash build.sh [OPTIONS]

Options:
  --op=NAME             Build specific operator (e.g., uniform)
  --soc=VERSION         Target SoC version $(printf "%s" "${SUPPORTED_SOCS}")
  --build-type=TYPE     Build type: Release (default), Debug 
  --run                 Run operator test after build
  --pkg                 Build package (RUN file)
  -j[N]                 Parallel build threads (default: $(nproc))
  -v, --verbose         Verbose output
  -h, --help            Show this help message

Examples:
  # Basic build
  bash build.sh

  # Build for Ascend 950
  bash build.sh --soc=ascend950 -j16

  # Build with package
  bash build.sh --pkg --soc=ascend950

  # Build specific operator
  bash build.sh --op=uniform --run
EOF
    exit 0
}

run_tests() {
    log_info "Running tests with ctest..."
    
    cd "$BUILD_DIR"
    
    # 构建ctest参数
    local ctest_args="--output-on-failure"
    
    # 并行执行
    local num_jobs=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
    ctest_args+=" -j${num_jobs}"
    log_info "Using ${num_jobs} parallel jobs"
    
    # 如果指定了具体的算子（不是all），添加过滤
    if [ -n "$BUILD_OP" ] && [ "$BUILD_OP" != "all" ]; then
        # 将逗号分隔的算子列表转换为正则表达式
        IFS=',' read -ra OPS <<< "$BUILD_OP"
        local regex=""
        for op in "${OPS[@]}"; do
            # 去除可能的空格
            op=$(echo "$op" | xargs)
            if [ -n "$regex" ]; then
                regex="${regex}|${op}"
            else
                regex="${op}"
            fi
        done
        # 添加测试过滤：匹配 test_算子名_example 格式
        ctest_args+=" -R ^test_(${regex})_example$"
        log_info "Test filter: ^test_(${regex})_example$"
    else
        log_info "Running all tests (no filter applied)"
    fi
    
    # 列出所有可用的测试（用于调试）
    log_info "Available tests:"
    ctest -N
    
    # 运行测试
    log_info "Running ctest with args: $ctest_args"
    ctest $ctest_args
    
    local test_result=$?
    
    cd "$SCRIPT_DIR"
    
    if [ $test_result -ne 0 ]; then
        log_error "Some tests failed"
        exit 1
    else
        log_success "All tests passed"
    fi
}

# 检查依赖项
check_dependencies() {
    log_info "Checking dependencies..."

    local missing_deps=0

    # 检查cmake
    if ! command -v cmake &> /dev/null; then
        log_error "cmake is not installed"
        missing_deps=$((missing_deps + 1))
    else
        local cmake_version=$(cmake --version | head -n1 | awk '{print $3}')
        log_success "cmake is installed (version: $cmake_version)"
    fi

    # 检查make
    if ! command -v make &> /dev/null; then
        log_warning "make is not installed, will use ninja or other generator"
    else
        local make_version=$(make --version | head -n1)
        log_success "make is installed ($make_version)"
    fi

    # 检查编译器 (gcc/g++)
    if ! command -v g++ &> /dev/null; then
        log_error "g++ is not installed"
        missing_deps=$((missing_deps + 1))
    else
        local gcc_version=$(g++ --version | head -n1)
        log_success "g++ is installed ($gcc_version)"
    fi

    # 检查ASC编译器
    if ! command -v bisheng &> /dev/null; then
        log_error "bisheng compiler (bisheng) is not installed"
        missing_deps=$((missing_deps + 1))
    else
        local asc_version=$(bisheng --version | head -n1)
        log_success "bisheng compiler is installed ($asc_version)"
    fi

    # 检查Python (用于测试脚本)
    if ! command -v python3 &> /dev/null && ! command -v python &> /dev/null; then
        log_warning "Python is not installed, some test scripts may not run"
    else
        local python_cmd="python3"
        if ! command -v python3 &> /dev/null; then
            python_cmd="python"
        fi
        local python_version=$($python_cmd --version 2>&1)
        log_success "Python is installed ($python_version)"
    fi

    if [ $missing_deps -gt 0 ]; then
        log_error "Missing $missing_deps required dependencies, please install and retry"
        exit 1
    fi

    log_success "All dependencies checked"
}

# 清理构建目录
clean_build() {
    log_info "Cleaning build directory..."
    if [ -d "$BUILD_DIR" ]; then
        rm -rf "$BUILD_DIR"
        log_success "Build directory cleaned"
    else
        log_warning "Build directory does not exist, nothing to clean"
    fi
}

# 解析命令行参数
parse_arguments() {
    for arg in "$@"; do
        case $arg in
            --op=*)
                BUILD_OP="${arg#*=}"
                ;;
            --run)
                RUN_TEST=ON
                ;;
            --pkg)
                ENABLE_PACKAGE=ON
                ;;
            --soc=*)
                SOC_VERSION="${arg#*=}"
                ;;
            -j*)
                THREAD_NUM="${arg#-j}"
                ;;
            --build-type=*)
                BUILD_TYPE="${arg#*=}"
                ;;
            -v|--verbose)
                VERBOSE="-v"
                ;;
            --help|-h)
                show_help
                ;;
            *)
                echo "Unknown option: $arg"
                echo "Use --help for usage information"
                exit 1
                ;;
        esac
    done
}

# 主函数
main() {
    #检查参数
    parse_arguments "$@"

    # 芯片版本验证
    if [ -n "$SOC_VERSION" ]; then
        if ! validate_soc_version "$SOC_VERSION"; then
            log_error "Unsupported SoC version: ${SOC_VERSION}"
            show_supported_socs
            exit 1
        else
            log_success "Supported SoC version: ${SOC_VERSION}"
        fi
    fi

    echo "=========================================="
    echo "Build Configuration:"
    echo "  BUILD_OP       = ${BUILD_OP:-all}"
    echo "  RUN_TEST       = ${RUN_TEST}"
    echo "  ENABLE_PACKAGE = ${ENABLE_PACKAGE}"
    echo "  SOC_VERSION    = ${SOC_VERSION:-not set}"
    echo "  BUILD_TYPE     = ${BUILD_TYPE}"
    echo "  THREAD_NUM     = ${THREAD_NUM}"
    echo "=========================================="

    # 验证必要参数
    if [ -z "$SOC_VERSION" ] && [ "$ENABLE_PACKAGE" = "ON" ]; then
        echo "Error: --soc is required for packaging"
        echo "Example: bash build.sh --pkg --soc=ascend950"
        exit 1
    fi

    # 依赖检查
    check_dependencies

    clean_build

    CMAKE_OPTIONS=""

    if [ -n "$BUILD_OP" ]; then
        # 如果指定了 --op，直接使用指定的值
        CMAKE_OPTIONS="${CMAKE_OPTIONS} -DCOMPILED_OPS=${BUILD_OP}"
        log_info "Building specified operator(s): ${BUILD_OP}"
    else
        # 如果没有指定 --op，设置 COMPILED_OPS=all 表示全量编译
        CMAKE_OPTIONS="${CMAKE_OPTIONS} -DCOMPILED_OPS=all"
        BUILD_OP="all"
        log_info "No operators specified, building all operators"
    fi

    # 打包选项
    if [ "$ENABLE_PACKAGE" = "ON" ]; then
        CMAKE_OPTIONS="${CMAKE_OPTIONS} -DENABLE_PACKAGE=ON"
    fi

    CMAKE_OPTIONS="${CMAKE_OPTIONS} -DBUILD_TEST=${RUN_TEST}"
    CMAKE_OPTIONS="${CMAKE_OPTIONS} -DCMAKE_BUILD_TYPE=${BUILD_TYPE}"
    CMAKE_OPTIONS="${CMAKE_OPTIONS} -DASCEND_HOME_PATH=${ASCEND_HOME_PATH}"
    CMAKE_OPTIONS="${CMAKE_OPTIONS} -DSOC_VERSION=${SOC_VERSION}"

    echo "CMake Options: ${CMAKE_OPTIONS}"

    # 执行 CMake 配置
    cmake -B ${BUILD_DIR} ${CMAKE_OPTIONS}

    # 执行构建
    cmake --build ${BUILD_DIR} -j${THREAD_NUM} ${VERBOSE}

    # 运行测试（如果指定）
    if [ "$RUN_TEST" = "ON" ]; then
        run_tests
    fi

    # 执行打包
    if [ "$ENABLE_PACKAGE" = "ON" ]; then
        echo ""
        echo "=============== Building Package ==============="
        cmake --build ${BUILD_DIR} --target package ${VERBOSE}
        echo ""
        echo "Package created successfully!"
        echo ""
        # 显示生成的 .run 文件路径
        if ls ${BUILD_OUT_DIR}/*.run 1> /dev/null 2>&1; then
            echo "Generated package(s):"
            ls -la ${BUILD_OUT_DIR}/*.run
        else
            echo "Looking for package files..."
            find ${BUILD_OUT_DIR} -name "*.run" -type f 2>/dev/null || echo "No .run files found in ${BUILD_OUT_DIR}"
        fi
    fi

    # 安装
    cmake --install ${BUILD_DIR}

    log_success "All operations completed!"
}

# 运行主函数
main "$@"