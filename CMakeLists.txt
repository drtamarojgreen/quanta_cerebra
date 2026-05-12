cmake_minimum_required(VERSION 3.10)
project(QuantaCerebra)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(src)

set(LIB_SOURCES
    # Core
    src/core/atlas_core.cpp
    src/core/atlas_region.cpp
    src/core/state_manager.cpp
    src/core/simulation_engine.cpp
    src/core/neurochemistry.cpp
    src/core/pathway_logic.cpp
    src/core/data_parsing_hub.cpp
    src/core/modeling_engine.cpp
    src/core/region_pool.cpp
    src/core/sample.cpp

    # IO
    src/io/json_parser.cpp
    src/io/yaml_parser.cpp
    src/io/xml_parser.cpp
    src/io/csv_parser.cpp
    src/io/serial_port.cpp
    src/io/input_source.cpp
    src/io/simulated_device.cpp
    src/io/config.cpp
    src/io/exporters.cpp

    # UI
    src/ui/interactive_ui.cpp
    src/ui/terminal_renderer.cpp
    src/ui/visual_themes.cpp
    src/ui/guided_tour.cpp
    src/ui/cli_handler.cpp
    src/ui/report_ui.cpp
    src/ui/app.cpp
    src/ui/ui_controller.cpp
    src/ui/color_theme.cpp

    # Visualization
    src/visualization/view_2d.cpp
    src/visualization/view_3d.cpp
    src/visualization/scene_renderer.cpp
    src/visualization/video_logic.cpp

    # Modules
    src/ai/ai.cpp
    src/analytics/analytics.cpp
    src/api/qc_api.cpp
    src/cloud/cloud.cpp
)

add_library(quantacerebra_lib ${LIB_SOURCES})

add_executable(QuantaCerebra src/main.cpp)
target_link_libraries(QuantaCerebra PRIVATE quantacerebra_lib)

# Tests integration
file(GLOB_RECURSE UNIT_TESTS "tests/unit/*.cpp")

set(TEST_EXECUTABLES "")

foreach(TEST_FILE ${UNIT_TESTS})
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    add_executable(${TEST_NAME} ${TEST_FILE})
    target_link_libraries(${TEST_NAME} PRIVATE quantacerebra_lib)
    list(APPEND TEST_EXECUTABLES ${TEST_NAME})
endforeach()

# BDD Runner
add_executable(bdd_runner tests/bdd/bdd_runner.cpp)
target_link_libraries(bdd_runner PRIVATE quantacerebra_lib)
list(APPEND TEST_EXECUTABLES bdd_runner)

# Data Parsing Hub Test (explicitly named for clarity)
if(NOT TARGET test_data_parsing_hub)
    add_executable(test_data_parsing_hub tests/unit/core/test_data_parsing_hub.cpp)
    target_link_libraries(test_data_parsing_hub PRIVATE quantacerebra_lib)
    list(APPEND TEST_EXECUTABLES test_data_parsing_hub)
endif()

# SDD Cards
add_executable(sdd_method_audit tests/sdd/cards/MethodAudit.cpp)
target_link_libraries(sdd_method_audit PRIVATE quantacerebra_lib)
list(APPEND TEST_EXECUTABLES sdd_method_audit)

add_executable(sdd_resilience_audit tests/sdd/cards/ResilienceAudit.cpp)
target_link_libraries(sdd_resilience_audit PRIVATE quantacerebra_lib)
list(APPEND TEST_EXECUTABLES sdd_resilience_audit)

add_executable(sdd_equipment_learning_audit tests/sdd/cards/EquipmentLearningAudit.cpp)
target_link_libraries(sdd_equipment_learning_audit PRIVATE quantacerebra_lib)
list(APPEND TEST_EXECUTABLES sdd_equipment_learning_audit)

add_executable(sdd_structural_audit tests/sdd/cards/StructuralAudit.cpp)
target_link_libraries(sdd_structural_audit PRIVATE quantacerebra_lib)
list(APPEND TEST_EXECUTABLES sdd_structural_audit)

# Enforce test binary locations
set_target_properties(${TEST_EXECUTABLES} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/tests/bin"
)


