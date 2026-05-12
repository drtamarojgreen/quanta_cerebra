#!/bin/bash
set -e

echo "QuantaCerebra E2E Full Workflow Test"

TEMP_DIR="tests/temp"
mkdir -p "$TEMP_DIR"

# 1. Verify basic execution
echo "Test: Basic help"
./QuantaCerebra --help > /dev/null

# 2. Test Report Mode with default template
echo "Test: Report mode (focused template)"
./QuantaCerebra --report --template focused > "$TEMP_DIR/report.txt"
if [ ! -s "$TEMP_DIR/report.txt" ]; then
    echo "Report is empty!"
    exit 1
fi

# 3. Test with custom Atlas
echo "Test: Custom atlas loading"
./QuantaCerebra --report --atlas data/builtin_atlas.json > "$TEMP_DIR/atlas_report.txt"
if [ ! -s "$TEMP_DIR/atlas_report.txt" ]; then
    echo "Atlas report is empty!"
    exit 1
fi

# 4. Test with JSON input
echo "Test: JSON input parsing"
echo '[{"timestamp_ms": 0, "brain_activity": [{"region": "prefrontal_cortex", "intensity": 0.5}]}]' > "$TEMP_DIR/test_input.json"
./QuantaCerebra --report --input "$TEMP_DIR/test_input.json" > "$TEMP_DIR/json_report.txt"
if [ ! -s "$TEMP_DIR/json_report.txt" ]; then
    echo "JSON report is empty!"
    exit 1
fi

# Cleanup
rm -f "$TEMP_DIR/report.txt" "$TEMP_DIR/atlas_report.txt" "$TEMP_DIR/test_input.json" "$TEMP_DIR/json_report.txt"

echo "E2E Workflow Test PASSED!"
