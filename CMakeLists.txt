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

# Enhancement 66: Sanitizer Support
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
if(ENABLE_ASAN)
    target_compile_options(QuantaCerebra PUBLIC -fsanitize=address)
    target_link_options(QuantaCerebra PUBLIC -fsanitize=address)
endif()
