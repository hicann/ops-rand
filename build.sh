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
BUILD_OPERATORS="all"
RUN_TEST=OFF
ENABLE_PACKAGE=OFF
SOC_VERSION="Ascend950"
THREAD_NUM=8
BUILD_TYPE=Release
VERBOSE=""
CMAKE_OPTIONS=""
TEST_TIMEOUT=300  # 默认测试超时时间（秒）

# 获取脚本所在目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

CORE_NUMS=$(cat /proc/cpuinfo | grep "processor" | wc -l)
CANN_3RD_LIB_PATH="${SCRIPT_DIR}/third_party"
SUPPORTED_SOCS=("Ascend950")

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

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

# SoC 名称标准化函数（首字母大写，其余小写）
normalize_soc_name() {
    local soc="$1"
    # 转换为小写，然后首字母大写
    echo "${soc}" | sed 's/.*/\L&/; s/^./\U&/; s/.$/\U&/'
}

# ==========================
# 帮助信息
# ==========================
show_help() {
    cat << EOF
Usage: $(basename "$0") [OPTIONS]

Options:
  --op=NAME              Build specific operator (e.g., stateless_random_uniform_v2)
  --soc=VERSION         Target SoC version $(printf "%s" "${SUPPORTED_SOCS}")
  --build-type=TYPE     Build type: Release (default), Debug
  --test-timeout=N      Test timeout in seconds (default: 300)
  --make_clean          Clean build artifacts"
  --run                 Run operator test after build
  --pkg                 Build package (RUN file)
  -j[N]                 Parallel build threads (default: $(nproc))
  -v, --verbose         Verbose output
  -h, --help            Show this help message

Examples:
  $(basename "$0")                                    # Build all operators (default 8 threads)
  $(basename "$0") --op=stateless_random_uniform_v2   # Build specific operator
  $(basename "$0") --op=op1,op2                       # Build multiple operators
  $(basename "$0") -j16                               # Build with 16 threads
  $(basename "$0") --run                              # Build all and run tests
  $(basename "$0") --op=stateless_random_uniform_v2 --run  # Build and run tests
  $(basename "$0") --pkg                              # Build package (default SoC: Ascend950)
  $(basename "$0") --op=stateless_random_uniform_v2 --pkg # Build and package
  $(basename "$0") --soc=Ascend950 --pkg              # Build package for Ascend950
  $(basename "$0") --soc=ascend950 --pkg              # Build package (lowercase also works)
  $(basename "$0") --test-timeout=600 --run           # Run tests with 600s timeout
  $(basename "$0") --make_clean                       # Clean build artifacts
EOF
    exit 0
}

run_tests() {
    log_info "Running tests (timeout: ${TEST_TIMEOUT}s)..."

    cd "$BUILD_DIR" || {
        log_error "Build directory not found: $BUILD_DIR"
        exit 1
    }

    # 检查测试可执行文件
    if [ ! -f "./tests/all_ops_test" ]; then
        log_error "Test executable not found: ./tests/all_ops_test"
        log_error "Please build the project first with: ./build.sh --run"
        exit 1
    fi

    log_info "Found test executable, starting..."

    # 临时禁用 set -e，手动处理错误
    set +e
    timeout -k 1s ${TEST_TIMEOUT}s ./tests/all_ops_test 2>&1
    test_result=$?
    set -e

    cd "$SCRIPT_DIR"

    if [ $test_result -ge 124 ]; then
        log_error "Test timeout (${TEST_TIMEOUT}s exceeded)"
        exit 1
    elif [ $test_result -ne 0 ]; then
        log_error "Some tests failed (exit code: $test_result)"
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
    if [ -d "$BUILD_DIR" ]; then
        rm -rf "$BUILD_DIR"
        log_success "build directory cleaned"
    else
        log_info "build directory does not exist, nothing to clean"
    fi
}

clean_build_out() {
    if [ -d "${BUILD_OUT_DIR}" ]; then
        rm -rf "${BUILD_OUT_DIR}"
        log_success "build_out directory cleaned"
    else
        log_info "build_out directory does not exist, nothing to clean"
    fi
}

# 解析命令行参数
parse_arguments() {
    while [[ $# -gt 0 ]]; do
        case $1 in
            --op=*)
                BUILD_OPERATORS="${1#*=}"
                shift
                ;;
            --run)
                RUN_TEST=ON
                shift
                ;;
            --pkg)
                ENABLE_PACKAGE=ON
                shift
                ;;
            --soc=*)
                # 提取 SoC 型号并标准化（支持小写输入）
                SOC_INPUT="${1#*=}"
                SOC_VERSION=$(normalize_soc_name "${SOC_INPUT}")
                shift
                ;;
            -j*)
                # 提取线程数
                if [[ "$1" == "-j" ]]; then
                    # -j N 的形式
                    if [[ -z "$2" ]]; then
                        log_error "Missing thread number after -j"
                        exit 1
                    fi
                    if [[ "$2" == -* ]]; then
                        log_error "Invalid thread number: $2 (did you mean -j$N?)"
                        exit 1
                    fi
                    THREAD_NUM="$2"
                    shift 2
                else
                    # -jN 的形式
                    THREAD_NUM="${1#-j}"
                    shift
                fi
                if [[ ! "$THREAD_NUM" =~ ^[0-9]+$ ]]; then
                    log_error "non-integer argument:$THREAD_NUM"
                    exit 1
                fi
                ;;
            --build-type=*)
                BUILD_TYPE="${1#*=}"
                shift
                ;;
            -v|--verbose)
                VERBOSE="-v"
                shift
                ;;
            --help|-h)
                show_help
                ;;
            --test-timeout=*)
                # 提取测试超时时间
                TEST_TIMEOUT="${1#*=}"
                shift
                ;;
            --make_clean)
                clean_build
                clean_build_out
                exit 0
                ;;
            *)
                log_error "Unknown option: $1"
                log_info "Use --help for usage information"
                exit 1
                ;;
        esac
    done
}
assemble_cmake_args() {
    if [ "$THREAD_NUM" -gt "$CORE_NUMS" ]; then
        log_info "compile thread num:$THREAD_NUM over core num:$CORE_NUMS, adjust to core num"
        THREAD_NUM=$CORE_NUMS
    fi

    if [[ -n "${BUILD_TYPE}" ]]; then
        if [[ "${BUILD_TYPE}" != "Release" && "${BUILD_TYPE}" != "Debug" ]]; then
        log_error "--build-type only support Release/Debug Mode"
        exit 1
        fi
    fi

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

    # 验证必要参数
    if [ -z "$SOC_VERSION" ] && [ "$ENABLE_PACKAGE" = "ON" ]; then
        echo "Error: --soc is required for packaging"
        echo "Example: bash build.sh --pkg --soc=ascend950"
        exit 1
    fi

    if [ "$BUILD_OPERATORS" != "all" ]; then
        CMAKE_OPTIONS="${CMAKE_OPTIONS} -DENABLED_OPERATORS=${BUILD_OPERATORS}"
        log_info "Building specified operator(s): ${BUILD_OPERATORS}"
    else
        log_info "No operators specified, building all operators"
    fi

    # 打包选项
    if [ "$ENABLE_PACKAGE" = "ON" ]; then
        CMAKE_OPTIONS="${CMAKE_OPTIONS} -DENABLE_PACKAGE=ON"
    fi

    CMAKE_OPTIONS="${CMAKE_OPTIONS} -DBUILD_TEST=${RUN_TEST}"
    CMAKE_OPTIONS="${CMAKE_OPTIONS} -DCMAKE_BUILD_TYPE=${BUILD_TYPE}"
    CMAKE_OPTIONS="${CMAKE_OPTIONS} -DSOC_VERSION=${SOC_VERSION}"
    CMAKE_OPTIONS="${CMAKE_OPTIONS} -DASCEND_HOME_PATH=${ASCEND_HOME_PATH}"
    CMAKE_OPTIONS="${CMAKE_OPTIONS} -DCANN_3RD_LIB_PATH=${CANN_3RD_LIB_PATH}"
}

# 主函数
main() {
    #检查参数
    parse_arguments "$@"

    assemble_cmake_args
    echo "CMake Options: ${CMAKE_OPTIONS}"

    # 依赖检查
    check_dependencies

    clean_build

    # 执行 CMake 配置
    cmake -B ${BUILD_DIR} ${CMAKE_OPTIONS}

    # 执行构建
    log_info "Compiling with ${THREAD_NUM} threads..."
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