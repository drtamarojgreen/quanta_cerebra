#!/bin/bash
set -e

echo "QuantaCerebra SDD Suite - Reliability and Resilience Equipment Learning Audit"

# Navigate to the directory containing the script
cd "$(dirname "$0")"

# Assuming binaries are in ../bin/ as per CMakeLists.txt or build/ as per execute_sorrel_audit.sh
# For the suite run, we expect them to be in the current path or ../bin
BIN_DIR="../bin"

# Helper function to run a test and check its exit code
run_test() {
    local test_name=$1
    local test_path="$BIN_DIR/$test_name"

    if [ ! -f "$test_path" ]; then
        # Fallback to local dir if not in ../bin
        test_path="./$test_name"
    fi

    if [ ! -f "$test_path" ]; then
        echo "ERROR: Test $test_name not found at $BIN_DIR or current directory."
        exit 1
    fi

    echo "Running Test: $test_name"
    "$test_path"
    echo "----------------------------------------------------------------"
}

# 1. Run Method Audit
run_test "sdd_method_audit"

# 2. Run Resilience Audit
run_test "sdd_resilience_audit"

# 3. Run Equipment Learning Audit
run_test "sdd_equipment_learning_audit"

# 4. Run Structural Audit
run_test "sdd_structural_audit"

echo "ALL SDD AUDITS PASSED!"
