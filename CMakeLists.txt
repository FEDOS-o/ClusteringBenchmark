cmake_minimum_required(VERSION 3.16)
project(ClusteringBenchmark VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_TESTS "Build unit tests" OFF)

set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)

add_library(clustering_lib STATIC
        ${SRC_DIR}/core/Graph.cpp
        ${SRC_DIR}/core/Cluster.cpp
        ${SRC_DIR}/algorithms/GreedyClustering.cpp
        ${SRC_DIR}/algorithms/LouvainClustering.cpp
        ${SRC_DIR}/algorithms/GLBClustering.cpp
        ${SRC_DIR}/metrics/MetricsCalculator.cpp
        ${SRC_DIR}/metrics/ClusteringMetrics.cpp
        ${SRC_DIR}/io/GraphLoader.cpp
        ${SRC_DIR}/io/GraphGenerator.cpp
        ${SRC_DIR}/io/ResultExporter.cpp
        ${SRC_DIR}/utils/Timer.cpp
        ${SRC_DIR}/utils/Logger.cpp
)

target_include_directories(clustering_lib PRIVATE ${INCLUDE_DIR})

add_executable(clustering_benchmark ${SRC_DIR}/main.cpp)
target_link_libraries(clustering_benchmark clustering_lib)
target_include_directories(clustering_benchmark PRIVATE ${INCLUDE_DIR})

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

message(STATUS "Project configured")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Tests: ${BUILD_TESTS}")