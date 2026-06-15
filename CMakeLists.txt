cmake_minimum_required(VERSION 3.16)
project(ClusteringBenchmark VERSION 1.0.0 LANGUAGES CXX)

# C++17 стандарт
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Опции сборки
option(BUILD_TESTS "Build unit tests" OFF)
option(ENABLE_OPENMP "Enable OpenMP for parallel algorithms" ON)

# Настройка путей
set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)

# Поиск зависимостей
find_package(OpenMP)
if(ENABLE_OPENMP AND OpenMP_CXX_FOUND)
    set(USE_OPENMP TRUE)
    message(STATUS "OpenMP found, enabling parallelization")
endif()

# Добавление библиотеки
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

if(USE_OPENMP)
    target_link_libraries(clustering_lib PUBLIC OpenMP::OpenMP_CXX)
endif()

# Добавление исполняемого файла
add_executable(clustering_benchmark ${SRC_DIR}/main.cpp)
target_link_libraries(clustering_benchmark clustering_lib)
target_include_directories(clustering_benchmark PRIVATE ${INCLUDE_DIR})

# Опциональные тесты
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Установка
install(TARGETS clustering_benchmark DESTINATION bin)
install(DIRECTORY data/ DESTINATION data)
install(DIRECTORY results/ DESTINATION results)

# Вывод информации
message(STATUS "Project configured:")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  OpenMP: ${USE_OPENMP}")
message(STATUS "  Tests: ${BUILD_TESTS}")