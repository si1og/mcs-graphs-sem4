cmake_minimum_required(VERSION 3.10)

project(lab1 CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

set(EIGEN_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../Eigen)

add_executable(main
    main.cpp
    universe.cpp
    multiset.cpp
    cliui.cpp
)

target_include_directories(main PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${EIGEN_INCLUDE_DIR}
)

include(CTest)
add_test(NAME run_project COMMAND main)
