add_library(core STATIC
    matrix.cpp
    graph.cpp
    generator_graph.cpp
)

target_include_directories(core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
