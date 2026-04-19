add_library(lab1_core STATIC
    matrix.cpp
    graph.cpp
    lab1.cpp
)

target_include_directories(lab1_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
