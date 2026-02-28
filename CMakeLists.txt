cmake_minimum_required(VERSION 3.16.0)

project(ops_rand)

# ========= 基本配置 =========
set(SOC_VERSION ascend950dt_9595)
set(ASCEND_CANN_PACKAGE_PATH $ENV{ASCEND_HOME_PATH})
set(RUN_MODE "npu" CACHE STRING "run mode: npu")
set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type Release/Debug" FORCE)
set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_LIST_DIR}/out" CACHE STRING "install path" FORCE)

  
# ========= 进入子目录 =========



set(ALL_SRC_FILES "")
set(OPS_RAND ops_rand)

find_package(ASC REQUIRED)

add_subdirectory(src)

project(${OPS_RAND} LANGUAGES ASC CXX)

add_library(${OPS_RAND} SHARED ${ALL_SRC_FILES})


set_source_files_properties(
${ALL_SRC_FILES}
PROPERTIES LANGUAGE ASC
)



target_include_directories(${OPS_RAND} PRIVATE
    ${ASCEND_HOME_PATH}/pkg_inc/op_common/
    ${ASCEND_HOME_PATH}/pkg_inc/base/
    ${ASCEND_HOME_PATH}/pkg_inc/
    ./include
)

target_compile_options(${OPS_RAND} PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-3101>
)

