cmake_minimum_required(VERSION 3.10)
project(QuantaCerebra)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(quantacerebra_lib
    src/json_logic.cpp
    src/video_logic.cpp
    src/config.cpp
    src/analytics.cpp
    src/exporters.cpp
    src/ai.cpp
    src/cloud.cpp
)

add_executable(QuantaCerebra src/main.cpp)
target_link_libraries(QuantaCerebra PRIVATE quantacerebra_lib)

# 200 Functional Unit Tests (20 files x 10 tests)
set(TEST_TARGETS
    test_core_modeling
    test_visualization
    test_data_io
    test_ux_interactivity
    test_performance
    test_ai_advanced
    test_dev_tooling
    test_ecosystem
    test_portability
    test_community_docs
    test_analytics
    test_education
    test_hardware
    test_cloud_distributed
    test_security_privacy
    test_api_sdk
    test_artistic_generative
    test_usability
    test_collaboration
    test_quality_testing
)

foreach(TF ${TEST_TARGETS})
    add_executable(${TF} tests/${TF}.cpp)
    target_link_libraries(${TF} PRIVATE quantacerebra_lib)
endforeach()
