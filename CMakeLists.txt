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

# Test targets
foreach(CAT RANGE 1 20)
    add_executable(cat_${CAT} tests/category_${CAT}.cpp)
    target_link_libraries(cat_${CAT} PRIVATE quantacerebra_lib)
endforeach()
