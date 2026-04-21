cmake_minimum_required(VERSION 3.10)
project(QuantaCerebra)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optimization flags (Enhancement 44 - partial)
if(MSVC)
    add_compile_options(/W4 /O2)
else()
    add_compile_options(-Wall -Wextra -O3 -march=native)
endif()

# Enhancement 72: Library target
add_library(quantacerebra_lib
    src/json_logic.cpp
    src/video_logic.cpp
    src/config.cpp
)

add_executable(QuantaCerebra src/main.cpp)
target_link_libraries(QuantaCerebra PRIVATE quantacerebra_lib)

# Enhancement 65: Benchmarking
add_executable(benchmark_suite src/main.cpp) # Use main as benchmark for now
target_link_libraries(benchmark_suite PRIVATE quantacerebra_lib)

# Category-based Unit Tests
add_executable(test_core tests/test_core.cpp)
target_link_libraries(test_core PRIVATE quantacerebra_lib)

add_executable(test_visualization tests/test_visualization.cpp)
target_link_libraries(test_visualization PRIVATE quantacerebra_lib)

add_executable(test_data_io tests/test_data_io.cpp)
target_link_libraries(test_data_io PRIVATE quantacerebra_lib)

add_executable(test_ux_perf tests/test_ux_perf.cpp)
target_link_libraries(test_ux_perf PRIVATE quantacerebra_lib)

add_executable(test_ai_integration tests/test_ai_integration.cpp)
target_link_libraries(test_ai_integration PRIVATE quantacerebra_lib)

add_executable(test_port_docs_analytics tests/test_port_docs_analytics.cpp)
target_link_libraries(test_port_docs_analytics PRIVATE quantacerebra_lib)

add_executable(test_access_cloud_security tests/test_access_cloud_security.cpp)
target_link_libraries(test_access_cloud_security PRIVATE quantacerebra_lib)

add_executable(test_advanced_features tests/test_advanced_features.cpp)
target_link_libraries(test_advanced_features PRIVATE quantacerebra_lib)

# Enhancement 66: Sanitizer Support
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
if(ENABLE_ASAN)
    target_compile_options(QuantaCerebra PUBLIC -fsanitize=address)
    target_link_options(QuantaCerebra PUBLIC -fsanitize=address)
endif()
