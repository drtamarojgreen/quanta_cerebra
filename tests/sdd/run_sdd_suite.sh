#!/bin/bash
set -e

echo "QuantaCerebra SDD Suite - Reliability and Resilience Equipment Learning Audit"

# 1. Run Method Audit
echo "Test: SDD Method Audit"
./sdd_method_audit

# 2. Run Resilience Audit
echo "Test: SDD Resilience Audit"
./sdd_resilience_audit

# 3. Run Equipment Learning Audit
echo "Test: SDD Equipment Learning Audit"
./sdd_equipment_learning_audit

echo "SDD Suite PASSED!"

