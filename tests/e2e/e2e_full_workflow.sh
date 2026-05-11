#!/bin/bash
set -e

echo "QuantaCerebra E2E Full Workflow Test"

# 1. Verify basic execution
echo "Test: Basic help"
./QuantaCerebra --help > /dev/null

# 2. Test Report Mode with default template
echo "Test: Report mode (focused template)"
./QuantaCerebra --report --template focused > report.txt
if [ ! -s report.txt ]; then
    echo "Report is empty!"
    exit 1
fi

# 3. Test with custom Atlas
echo "Test: Custom atlas loading"
./QuantaCerebra --report --atlas data/builtin_atlas.json > atlas_report.txt
if [ ! -s atlas_report.txt ]; then
    echo "Atlas report is empty!"
    exit 1
fi

# 4. Test with JSON input
echo "Test: JSON input parsing"
echo '[{"timestamp_ms": 0, "brain_activity": [{"region": "prefrontal_cortex", "intensity": 0.5}]}]' > test_input.json
./QuantaCerebra --report --input test_input.json > json_report.txt
if [ ! -s json_report.txt ]; then
    echo "JSON report is empty!"
    exit 1
fi

# Cleanup
rm -f report.txt atlas_report.txt test_input.json json_report.txt

echo "E2E Workflow Test PASSED!"
