#!/bin/bash
set -e

# QuantaCerebra Standard Operating Procedure: Reliability & Resilience Audit
# Format: Sorrel Driven Development (SDD)

echo "----------------------------------------------------------------"
echo "CEREBRA EQUIPMENT LEARNING AUDIT - INITIALIZING"
echo "----------------------------------------------------------------"

# 1. Prepare Environment
mkdir -p tests/sdd/build
SRC_DIR="src"
SDD_DIR="tests/sdd"
BUILD_DIR="tests/sdd/build"

# 2. Compile Core Library (Required for Audit Tools)
echo "[1/4] Compiling Cerebra Core Library..."
g++ -std=c++17 -Isrc -c \
    $SRC_DIR/core/*.cpp \
    $SRC_DIR/io/*.cpp \
    $SRC_DIR/ui/*.cpp \
    $SRC_DIR/visualization/*.cpp \
    $SRC_DIR/ai/*.cpp \
    $SRC_DIR/analytics/*.cpp \
    $SRC_DIR/api/*.cpp \
    $SRC_DIR/cloud/*.cpp 
mv *.o $BUILD_DIR/

# 3. Compile Audit Cards
echo "[2/4] Compiling Sorrel Audit Cards..."
g++ -std=c++17 -Isrc -I$SDD_DIR/cpp/util \
    $SDD_DIR/cards/MethodAudit.cpp $BUILD_DIR/*.o -o $BUILD_DIR/method_audit
g++ -std=c++17 -Isrc -I$SDD_DIR/cpp/util \
    $SDD_DIR/cards/ResilienceAudit.cpp $BUILD_DIR/*.o -o $BUILD_DIR/resilience_audit
g++ -std=c++17 -Isrc -I$SDD_DIR/cpp/util \
    $SDD_DIR/cards/EquipmentLearningAudit.cpp $BUILD_DIR/*.o -o $BUILD_DIR/equipment_learning_audit
g++ -std=c++17 -Isrc -I$SDD_DIR/cpp/util \
    $SDD_DIR/cards/StructuralAudit.cpp $BUILD_DIR/*.o -o $BUILD_DIR/structural_audit

# 4. Execute Audit Suite
echo "[3/4] Executing Full Audit Suite..."

echo ">> Running: Method Audit"
$BUILD_DIR/method_audit

echo ">> Running: Resilience Audit"
$BUILD_DIR/resilience_audit

echo ">> Running: Equipment Learning Audit"
$BUILD_DIR/equipment_learning_audit

echo ">> Running: Structural Audit"
$BUILD_DIR/structural_audit

# 5. Finalize
echo "[4/4] Audit Complete."
echo "----------------------------------------------------------------"
echo "EQUIPMENT STATUS: COMPLIANT WITH SO-RR-EL-2026"
echo "----------------------------------------------------------------"
