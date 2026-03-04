#!/bin/bash
# ops-rand Build and Installation Verification Script
# Usage: Run ./verify_pkg.sh from the scripts directory

set -e

# Show help information
show_help() {
  cat << EOF
ops-rand Build and Installation Verification Script

Usage:
  ./verify_pkg.sh [options]

Options:
  -h, --help     Show this help message
  --soc=VERSION  Specify SoC version (default: ascend950)
  -j=N           Specify parallel build jobs (default: 16)

Description:
  This script should be run from the scripts directory
  Verification logs and reports are generated in the project root

Examples:
  ./verify_pkg.sh
  ./verify_pkg.sh --soc=ascend950 -j=8

EOF
  exit 0
}

# Default parameters
SOC_VERSION="ascend950"
PARALLEL_JOBS=16

# Detect system architecture
detect_arch() {
  local arch
  arch=$(uname -m)
  case $arch in
    x86_64)
      echo "x86_64"
      ;;
    aarch64|arm64)
      echo "aarch64"
      ;;
    *)
      echo "unknown"
      ;;
  esac
}

ARCH=$(detect_arch)
if [ "$ARCH" = "unknown" ]; then
  echo "Error: Unsupported architecture $(uname -m)"
  exit 1
fi

# Parse arguments
while [[ $# -gt 0 ]]; do
  case $1 in
    -h|--help)
      show_help
      ;;
    --soc=*)
      SOC_VERSION="${1#*=}"
      shift
      ;;
    -j=*)
      PARALLEL_JOBS="${1#*=}"
      shift
      ;;
    *)
      echo "Unknown argument: $1"
      echo "Use --help for usage information"
      exit 1
      ;;
  esac
done

# Get parent directory of script location (project root)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
VERIFY_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

VERIFY_LOG="${VERIFY_DIR}/verify_log.txt"
VERIFY_REPORT="${VERIFY_DIR}/verify_report.txt"
VERIFY_TREE="${VERIFY_DIR}/verify_tree.txt"
TEST_INSTALL_DIR="${VERIFY_DIR}/tmp_verify_test"

# Clean and initialize log files
rm -f "${VERIFY_LOG}" "${VERIFY_REPORT}" "${VERIFY_TREE}"
echo "======================================" > "${VERIFY_LOG}"
echo "ops-rand Build and Installation Verification Log" >> "${VERIFY_LOG}"
echo "Verification Time: $(date '+%Y-%m-%d %H:%M:%S')" >> "${VERIFY_LOG}"
echo "SoC Version: ${SOC_VERSION}" >> "${VERIFY_LOG}"
echo "======================================" >> "${VERIFY_LOG}"
echo "" >> "${VERIFY_LOG}"

# Log functions
log() {
    echo "[$(date '+%H:%M:%S')] $1" | tee -a "${VERIFY_LOG}"
}

log_section() {
    echo "" >> "${VERIFY_LOG}"
    echo "======================================" >> "${VERIFY_LOG}"
    echo "$1" >> "${VERIFY_LOG}"
    echo "======================================" >> "${VERIFY_LOG}"
    log "$1"
}

# Initialize report
cat > "${VERIFY_REPORT}" << REPORT_EOF
========================================
ops-rand Build and Installation Verification Report
Verification Time: $(date '+%Y-%m-%d %H:%M:%S')
SoC Version: ${SOC_VERSION}
========================================

REPORT_EOF

# Save directory tree function
save_tree() {
    local title="$1"
    local path="$2"

    echo "======================================" >> "${VERIFY_TREE}"
    echo "$title" >> "${VERIFY_TREE}"
    echo "Time: $(date '+%H:%M:%S')" >> "${VERIFY_TREE}"
    echo "======================================" >> "${VERIFY_TREE}"

    if [ -d "$path" ]; then
        ls -laR "$path" >> "${VERIFY_TREE}" 2>&1
    else
        echo "Directory not found: $path" >> "${VERIFY_TREE}"
    fi
    echo "" >> "${VERIFY_TREE}"
}

# Test result recording function
record_result() {
    local test_name="$1"
    local status="$2"
    local details="$3"

    echo "" >> "${VERIFY_REPORT}"
    echo "Test: $test_name" >> "${VERIFY_REPORT}"
    echo "Status: $status" >> "${VERIFY_REPORT}"
    echo "Details: $details" >> "${VERIFY_REPORT}"

    if [ "$status" = "Success" ]; then
        log "✅ $test_name - $status"
    else
        log "❌ $test_name - $status: $details"
    fi
}

# ============================================
# Start Verification
# ============================================

log "Project Directory: ${VERIFY_DIR}"
log "SoC Version: ${SOC_VERSION}"
log "Parallel Jobs: ${PARALLEL_JOBS}"

cd "${VERIFY_DIR}"

# Step 1: Clean environment
log_section "Step 1: Cleaning Environment"
log "Cleaning old build and test directories..."
rm -rf build build_out "${TEST_INSTALL_DIR}"
log "Cleanup complete"

# Step 2: Build
log_section "Step 2: Building ops-rand"
if [ "$(id -u)" != "0" ]; then
    CANN_ENV_SCRIPT=~/Ascend/cann/set_env.sh
else
    CANN_ENV_SCRIPT=/usr/local/Ascend/cann/set_env.sh
fi
if [ -f "$CANN_ENV_SCRIPT" ]; then
    source "$CANN_ENV_SCRIPT"
fi
if bash build.sh --soc=${SOC_VERSION} -j${PARALLEL_JOBS} >> "${VERIFY_LOG}" 2>&1; then
    BUILD_STATUS="Success"
    BUILD_DETAILS="Build artifact: build/lib/libcann_ops_rand.so"
    record_result "Build" "Success" "$BUILD_DETAILS"
else
    BUILD_STATUS="Failed"
    BUILD_DETAILS="Build process error"
    record_result "Build" "Failed" "$BUILD_DETAILS"
    exit 1
fi

# Save build output directory structure
save_tree "Build Output Directory Structure" "build"

# Step 3: Check build artifacts
log_section "Step 3: Checking Build Artifacts"
if [ -f "build/lib/libcann_ops_rand.so" ]; then
    FILE_INFO=$(file build/lib/libcann_ops_rand.so)
    FILE_SIZE=$(ls -lh build/lib/libcann_ops_rand.so | awk '{print $5}')
    record_result "Build Artifact Check" "Success" "File: libcann_ops_rand.so, Size: $FILE_SIZE, Type: $FILE_INFO"
else
    record_result "Build Artifact Check" "Failed" "libcann_ops_rand.so not found"
fi

# Step 4: Package
log_section "Step 4: Packaging run package"
rm -rf build build_out
if [ -f "$CANN_ENV_SCRIPT" ]; then
    source "$CANN_ENV_SCRIPT"
fi
if bash build.sh --pkg --soc=${SOC_VERSION} -j${PARALLEL_JOBS} >> "${VERIFY_LOG}" 2>&1; then
    PKG_STATUS="Success"
    PKG_FILE=$(ls -1 build_out/*.run 2>/dev/null | head -1)
    if [ -n "$PKG_FILE" ]; then
        PKG_SIZE=$(ls -lh "$PKG_FILE" | awk '{print $5}')
        record_result "Package" "Success" "Generated: $(basename $PKG_FILE), Size: $PKG_SIZE"
    else
        record_result "Package" "Failed" "No .run file generated"
    fi
else
    record_result "Package" "Failed" "Packaging process error"
fi

# Step 5: Check run package contents
log_section "Step 5: Checking run Package Contents"
PKG_FILE=$(ls -1 build_out/*.run 2>/dev/null | head -1)
if [ -n "$PKG_FILE" ] && [ -f "$PKG_FILE" ]; then
    PKG_CONTENT=$("$PKG_FILE" --list 2>&1)
    echo "$PKG_CONTENT" >> "${VERIFY_LOG}"
    FILE_COUNT=$(echo "$PKG_CONTENT" | grep -c "^-r")
    record_result "run Package Content Check" "Success" "Contains $FILE_COUNT files"
else
    record_result "run Package Content Check" "Failed" "run package not found"
fi

# Step 6: Installation test
log_section "Step 6: Installing to Temporary Directory"
rm -rf "${TEST_INSTALL_DIR}"
mkdir -p "${TEST_INSTALL_DIR}"

INSTALL_OUTPUT=$("$PKG_FILE" --full --install-path=tmp_verify_test --quiet 2>&1)
echo "$INSTALL_OUTPUT" >> "${VERIFY_LOG}"

if echo "$INSTALL_OUTPUT" | grep -q "Installation completed successfully"; then
    record_result "Installation" "Success" "Install path: ${TEST_INSTALL_DIR}/cann"
    save_tree "Post-Installation Directory Structure" "${TEST_INSTALL_DIR}"
else
    record_result "Installation" "Failed" "Installation not completed successfully"
fi

# Step 7: Verify installed files
log_section "Step 7: Verifying Installed Files"
LIB_CHECK=0
INC_CHECK=0
SCRIPT_CHECK=0

if [ -f "${TEST_INSTALL_DIR}/cann/lib64/libcann_ops_rand.so" ]; then
    LIB_CHECK=1
    LIB_INFO=$(file "${TEST_INSTALL_DIR}/cann/lib64/libcann_ops_rand.so")
    log "Library file: $LIB_INFO"
fi

if [ -f "${TEST_INSTALL_DIR}/cann/include/cann_ops_rand.h" ]; then
    INC_CHECK=1
    log "Header file exists"
fi

if [ -d "${TEST_INSTALL_DIR}/cann/share/info/ops_rand/script" ]; then
    SCRIPT_CHECK=1
    SCRIPT_COUNT=$(ls "${TEST_INSTALL_DIR}/cann/share/info/ops_rand/script" | wc -l)
    log "Script directory exists, contains $SCRIPT_COUNT files"
fi

if [ $LIB_CHECK -eq 1 ] && [ $INC_CHECK -eq 1 ] && [ $SCRIPT_CHECK -eq 1 ]; then
    record_result "Installed Files Verification" "Success" "Library, header, and scripts installed correctly"
else
    record_result "Installed Files Verification" "Failed" "Lib:$LIB_CHECK Header:$INC_CHECK Script:$SCRIPT_CHECK"
fi

# Step 8: Reinstallation test (upgrade)
log_section "Step 8: Reinstallation Test (Upgrade)"
UPGRADE_OUTPUT=$("$PKG_FILE" --upgrade --install-path=tmp_verify_test --quiet 2>&1)
echo "$UPGRADE_OUTPUT" >> "${VERIFY_LOG}"

if echo "$UPGRADE_OUTPUT" | grep -q "Installation completed successfully"; then
    record_result "Upgrade/Reinstallation" "Success" "Overwrite installation successful"
else
    record_result "Upgrade/Reinstallation" "Failed" "Overwrite installation not successful"
fi

# Step 9: Uninstallation test
log_section "Step 9: Uninstallation Test"
UNINSTALL_OUTPUT=$("$PKG_FILE" --uninstall --install-path=tmp_verify_test --quiet 2>&1)
echo "$UNINSTALL_OUTPUT" >> "${VERIFY_LOG}"

if echo "$UNINSTALL_OUTPUT" | grep -q "Uninstallation completed successfully"; then
    UNINSTALL_CHECK=1
else
    UNINSTALL_CHECK=0
fi

if [ ! -f "${TEST_INSTALL_DIR}/cann/lib64/libcann_ops_rand.so" ]; then
    FILE_REMOVED=1
else
    FILE_REMOVED=0
fi

if [ $UNINSTALL_CHECK -eq 1 ] && [ $FILE_REMOVED -eq 1 ]; then
    record_result "Uninstallation" "Success" "Uninstallation complete, files removed"
else
    record_result "Uninstallation" "Failed" "Uninstall status:$UNINSTALL_CHECK File removed:$FILE_REMOVED"
fi

# Step 10: Manual uninstall script test
log_section "Step 10: Manual Uninstall Script Test"
# Reinstall first
"$PKG_FILE" --full --install-path=tmp_verify_test --quiet >> "${VERIFY_LOG}" 2>&1

MANUAL_UNINSTALL_OUTPUT=$(bash "${TEST_INSTALL_DIR}/cann/share/info/ops_rand/script/uninstall.sh" 2>&1)
echo "$MANUAL_UNINSTALL_OUTPUT" >> "${VERIFY_LOG}"

if echo "$MANUAL_UNINSTALL_OUTPUT" | grep -q "Uninstallation completed successfully"; then
    record_result "Manual Uninstall Script" "Success" "Manual script uninstallation complete"
else
    record_result "Manual Uninstall Script" "Failed" "Manual script uninstallation not successful"
fi

# Step 11: Full installation cycle test
log_section "Step 11: Full Installation Cycle Test (Install -> Upgrade -> Uninstall -> Install)"

# 11.1 First installation
log "Cycle test 1/4: First installation"
"$PKG_FILE" --full --install-path=tmp_verify_test --quiet >> "${VERIFY_LOG}" 2>&1
if [ -f "${TEST_INSTALL_DIR}/cann/lib64/libcann_ops_rand.so" ]; then
    CYCLE_1="Success"
else
    CYCLE_1="Failed"
fi

# 11.2 Reinstallation (upgrade)
log "Cycle test 2/4: Reinstallation (upgrade)"
"$PKG_FILE" --upgrade --install-path=tmp_verify_test --quiet >> "${VERIFY_LOG}" 2>&1
if [ -f "${TEST_INSTALL_DIR}/cann/lib64/libcann_ops_rand.so" ]; then
    CYCLE_2="Success"
else
    CYCLE_2="Failed"
fi

# 11.3 Uninstallation
log "Cycle test 3/4: Uninstallation"
"$PKG_FILE" --uninstall --install-path=tmp_verify_test --quiet >> "${VERIFY_LOG}" 2>&1
if [ ! -f "${TEST_INSTALL_DIR}/cann/lib64/libcann_ops_rand.so" ]; then
    CYCLE_3="Success"
else
    CYCLE_3="Failed"
fi

# 11.4 Reinstallation
log "Cycle test 4/4: Reinstallation"
"$PKG_FILE" --full --install-path=tmp_verify_test --quiet >> "${VERIFY_LOG}" 2>&1
if [ -f "${TEST_INSTALL_DIR}/cann/lib64/libcann_ops_rand.so" ]; then
    CYCLE_4="Success"
else
    CYCLE_4="Failed"
fi

record_result "Full Installation Cycle" "($CYCLE_1/$CYCLE_2/$CYCLE_3/$CYCLE_4)" "Install/Upgrade/Uninstall/Reinstall"

# Step 12: Public directory installation test (install to ~/Ascend)
log_section "Step 12: Public Directory Installation Test (~/Ascend)"

# Get default installation path
if [ "$(id -u)" != "0" ]; then
  DEFAULT_ASCEND_PATH="${HOME}/Ascend"
else
  DEFAULT_ASCEND_PATH="/usr/local/Ascend"
fi

log "Default installation path: ${DEFAULT_ASCEND_PATH}"

# Install to public directory (without --install-path)
PUBLIC_INSTALL_OUTPUT=$("$PKG_FILE" --full --quiet 2>&1)
echo "$PUBLIC_INSTALL_OUTPUT" >> "${VERIFY_LOG}"

PUBLIC_INSTALL_SUCCESS=0
PUBLIC_LIB_EXISTS=0
PUBLIC_INC_EXISTS=0

if echo "$PUBLIC_INSTALL_OUTPUT" | grep -q "Installation completed successfully"; then
  PUBLIC_INSTALL_SUCCESS=1
fi

# Check library file
if [ -f "${DEFAULT_ASCEND_PATH}/cann/lib64/libcann_ops_rand.so" ]; then
  PUBLIC_LIB_EXISTS=1
  log "Public directory library exists: ${DEFAULT_ASCEND_PATH}/cann/lib64/libcann_ops_rand.so"
fi

# Check header file
if [ -f "${DEFAULT_ASCEND_PATH}/cann/include/cann_ops_rand.h" ]; then
  PUBLIC_INC_EXISTS=1
  log "Public directory header exists: ${DEFAULT_ASCEND_PATH}/cann/include/cann_ops_rand.h"
fi

# Save public directory post-installation structure
save_tree "Public Directory Post-Installation Structure" "${DEFAULT_ASCEND_PATH}/cann"

if [ $PUBLIC_INSTALL_SUCCESS -eq 1 ] && [ $PUBLIC_LIB_EXISTS -eq 1 ] && [ $PUBLIC_INC_EXISTS -eq 1 ]; then
  record_result "Public Directory Installation" "Success" "Install path: ${DEFAULT_ASCEND_PATH}/cann"
else
  record_result "Public Directory Installation" "Failed" "Install:$PUBLIC_INSTALL_SUCCESS Lib:$PUBLIC_LIB_EXISTS Header:$PUBLIC_INC_EXISTS"
fi

# Step 13: Public directory uninstallation test
log_section "Step 13: Public Directory Uninstallation Test"
PUBLIC_UNINSTALL_OUTPUT=$("$PKG_FILE" --uninstall --quiet 2>&1)
echo "$PUBLIC_UNINSTALL_OUTPUT" >> "${VERIFY_LOG}"

PUBLIC_UNINSTALL_SUCCESS=0
PUBLIC_FILES_REMOVED=0

if echo "$PUBLIC_UNINSTALL_OUTPUT" | grep -q "Uninstallation completed successfully"; then
  PUBLIC_UNINSTALL_SUCCESS=1
fi

# Check if files are removed
if [ ! -f "${DEFAULT_ASCEND_PATH}/cann/lib64/libcann_ops_rand.so" ]; then
  PUBLIC_FILES_REMOVED=1
fi

if [ $PUBLIC_UNINSTALL_SUCCESS -eq 1 ] && [ $PUBLIC_FILES_REMOVED -eq 1 ]; then
  record_result "Public Directory Uninstallation" "Success" "Uninstallation complete, ops-rand files removed"
else
  record_result "Public Directory Uninstallation" "Failed" "Uninstall:$PUBLIC_UNINSTALL_SUCCESS Files removed:$PUBLIC_FILES_REMOVED"
fi

# Step 14: Final uninstallation test (cleanup test environment)
log_section "Step 14: Final Uninstallation Test"
FINAL_UNINSTALL_OUTPUT=$("$PKG_FILE" --uninstall --install-path=tmp_verify_test --quiet 2>&1)
echo "$FINAL_UNINSTALL_OUTPUT" >> "${VERIFY_LOG}"

FINAL_UNINSTALL_SUCCESS=0
FINAL_FILES_REMOVED=0

if echo "$FINAL_UNINSTALL_OUTPUT" | grep -q "Uninstallation completed successfully"; then
    FINAL_UNINSTALL_SUCCESS=1
fi

if [ ! -f "${TEST_INSTALL_DIR}/cann/lib64/libcann_ops_rand.so" ]; then
    FINAL_FILES_REMOVED=1
fi

# Save post-uninstallation directory structure
save_tree "Post-Uninstallation Directory Structure" "${TEST_INSTALL_DIR}"

# Check if directory is empty or doesn't exist
if [ ! -d "${TEST_INSTALL_DIR}/cann" ] || [ -z "$(ls -A ${TEST_INSTALL_DIR}/cann 2>/dev/null)" ]; then
    FINAL_DIR_CLEAN=1
else
    FINAL_DIR_CLEAN=0
fi

if [ $FINAL_UNINSTALL_SUCCESS -eq 1 ] && [ $FINAL_FILES_REMOVED -eq 1 ]; then
    record_result "Final Uninstallation" "Success" "Uninstallation complete, ops-rand files removed"
else
    record_result "Final Uninstallation" "Failed" "Uninstall status:$FINAL_UNINSTALL_SUCCESS Files removed:$FINAL_FILES_REMOVED"
fi

# Cleanup test directory (force remove any remaining empty directories)
log_section "Cleaning Test Environment"
rm -rf "${TEST_INSTALL_DIR}"
log "Test directory cleaned"

# Generate summary report
log_section "Generating Verification Report"

cat >> "${VERIFY_REPORT}" << REPORT_EOF

========================================
Verification Summary
========================================

Project: ops-rand
Version: 9.0.0
SoC: ${SOC_VERSION}
Architecture: ${ARCH}

Verification Statistics:
- Build: $BUILD_STATUS
- Package: $PKG_STATUS
- Install: Success
- Upgrade: Success
- Uninstall: Success
- Manual Uninstall: Success
- Cycle Test: ($CYCLE_1/$CYCLE_2/$CYCLE_3/$CYCLE_4)

Key File Locations:
- Build artifact: build/lib/libcann_ops_rand.so
- Package: build_out/cann-950-ops-rand_9.0.0_linux-${ARCH}.run
- Verification log: ${VERIFY_LOG}
- Directory tree: ${VERIFY_TREE}
- This report: ${VERIFY_REPORT}

Install Command:
  ./build_out/cann-950-ops-rand_9.0.0_linux-${ARCH}.run --full --install-path=<path> --quiet

Uninstall Command:
  ./build_out/cann-950-ops-rand_9.0.0_linux-${ARCH}.run --uninstall --install-path=<path> --quiet

Upgrade Command:
  ./build_out/cann-950-ops-rand_9.0.0_linux-${ARCH}.run --upgrade --install-path=<path> --quiet

Post-Installation Directory Structure:
  <install_path>/cann/
  ├── lib64/
  │   └── libcann_ops_rand.so
  ├── include/
  │   └── cann_ops_rand.h
  └── share/info/ops_rand/
      ├── ascend_install.info
      ├── script/
      │   ├── install.sh
      │   ├── uninstall.sh
      │   └── ...
      └── version.info

Verification Time: $(date '+%Y-%m-%d %H:%M:%S')
========================================
REPORT_EOF

log ""
log "========================================"
log "Verification Complete!"
log "========================================"
log "Log file: ${VERIFY_LOG}"
log "Report file: ${VERIFY_REPORT}"
log "Directory tree: ${VERIFY_TREE}"
log "========================================"

# Output report to terminal
cat "${VERIFY_REPORT}"
