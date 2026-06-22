#include "core/Graph.hpp"
#include "core/Cluster.hpp"
#include "algorithms/GreedyClustering.hpp"
#include "algorithms/LouvainClustering.hpp"
#include "algorithms/GLBClustering.hpp"
#include "metrics/MetricsCalculator.hpp"
#include "io/GraphLoader.hpp"
#include "io/GraphGenerator.hpp"
#include "io/ResultExporter.hpp"
#include "utils/Timer.hpp"
#include "utils/Logger.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <sstream>
#include <iomanip>

using namespace clustering;

struct CommandLineArgs {
    std::string inputFile = "";
    std::string outputFile = "results/experiment_results.csv";
    std::vector<std::string> algorithms = {"greedy", "glb"};
    int maxClusterSize = 64;
    int localIters = 3;
    bool enforceWarpMultiple = true;
    bool verbose = false;

    // Генерация сеток
    bool generateGrid = false;
    int gridSize = 100;
    bool generate3D = false;
    int gridDepth = 10;

    // Генерация случайных графов
    bool generateRandom = false;
    int randomN = 1000;
    double randomP = 0.01;
};

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [options]\n\n"
              << "INPUT OPTIONS:\n"
              << "  -i, --input FILE       Input graph file (CSV format)\n"
              << "  -g, --generate-grid N  Generate NxN grid graph\n"
              << "  -3, --generate-3d      Generate 3D grid (requires -g and --depth)\n"
              << "  --depth N              Depth for 3D grid (default: 10)\n"
              << "  -r, --random N P       Generate random Erdos-Renyi graph\n"
              << "                          N = vertices, P = edge probability (0.0-1.0)\n"
              << "\n"
              << "ALGORITHM OPTIONS:\n"
              << "  -a, --algorithms LIST  Comma-separated: greedy,louvain,glb (default: greedy,glb)\n"
              << "  -s, --max-size N       Maximum cluster size (default: 64)\n"
              << "  -l, --local-iters N    Local optimization iterations for GLB (default: 3)\n"
              << "  --no-warp-align        Disable warp alignment (32 multiple)\n"
              << "\n"
              << "OUTPUT OPTIONS:\n"
              << "  -o, --output FILE      Output CSV file (default: results/experiment_results.csv)\n"
              << "  -v, --verbose          Verbose output with detailed metrics\n"
              << "  -h, --help             Show this help\n"
              << "\n"
              << "EXAMPLES:\n"
              << "  " << programName << " -g 100 -a greedy,glb\n"
              << "  " << programName << " -r 1000 0.01 -a greedy,louvain,glb -v\n"
              << "  " << programName << " -g 20 -3 --depth 20 -a glb\n"
              << "  " << programName << " -i data/graph.csv -o results/my_results.csv\n";
}

CommandLineArgs parseArgs(int argc, char* argv[]) {
    CommandLineArgs args;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            exit(0);
        }
        else if (arg == "-i" || arg == "--input") {
            if (i + 1 < argc) args.inputFile = argv[++i];
        }
        else if (arg == "-o" || arg == "--output") {
            if (i + 1 < argc) args.outputFile = argv[++i];
        }
        else if (arg == "-a" || arg == "--algorithms") {
            if (i + 1 < argc) {
                std::string algos = argv[++i];
                args.algorithms.clear();
                std::stringstream ss(algos);
                std::string item;
                while (std::getline(ss, item, ',')) {
                    args.algorithms.push_back(item);
                }
            }
        }
        else if (arg == "-s" || arg == "--max-size") {
            if (i + 1 < argc) args.maxClusterSize = std::stoi(argv[++i]);
        }
        else if (arg == "-l" || arg == "--local-iters") {
            if (i + 1 < argc) args.localIters = std::stoi(argv[++i]);
        }
        else if (arg == "--no-warp-align") {
            args.enforceWarpMultiple = false;
        }
        else if (arg == "-g" || arg == "--generate-grid") {
            if (i + 1 < argc) {
                args.generateGrid = true;
                args.gridSize = std::stoi(argv[++i]);
            }
        }
        else if (arg == "-3" || arg == "--generate-3d") {
            args.generate3D = true;
        }
        else if (arg == "--depth") {
            if (i + 1 < argc) args.gridDepth = std::stoi(argv[++i]);
        }
        else if (arg == "-r" || arg == "--random") {
            args.generateRandom = true;
            if (i + 2 < argc) {
                args.randomN = std::stoi(argv[++i]);
                args.randomP = std::stod(argv[++i]);
            }
        }
        else if (arg == "-v" || arg == "--verbose") {
            args.verbose = true;
        }
        else {
            std::cerr << "Unknown option: " << arg << "\n";
            printUsage(argv[0]);
            exit(1);
        }
    }

    return args;
}

AlgorithmPtr createAlgorithm(const std::string& name) {
    if (name == "greedy" || name == "Greedy") {
        return std::make_unique<GreedyClustering>();
    } else if (name == "louvain" || name == "Louvain") {
        return std::make_unique<LouvainClustering>();
    } else if (name == "glb" || name == "GLB") {
        return std::make_unique<GLBClustering>();
    } else {
        throw std::runtime_error("Unknown algorithm: " + name);
    }
}

void printDetailedMetrics(const ExperimentResult& result) {
    const auto& m = result.metrics;

    std::cout << "\n";
    std::cout << "+-------------------------------------------------------+\n";
    std::cout << "|              DETAILED METRICS                         |\n";
    std::cout << "+-------------------------------------------------------+\n";

    // Базовые метрики
    std::cout << "| Clusters:          " << std::setw(22) << m.numClusters << " |\n";
    std::cout << "| Vertices:          " << std::setw(22) << m.numVertices << " |\n";
    std::cout << "| Edges:             " << std::setw(22) << m.numEdges << " |\n";
    std::cout << "+-------------------------------------------------------+\n";

    // Размеры кластеров
    std::cout << "| Min cluster size:  " << std::setw(22) << std::fixed << std::setprecision(2) << m.minClusterSize << " |\n";
    std::cout << "| Max cluster size:  " << std::setw(22) << m.maxClusterSize << " |\n";
    std::cout << "| Mean cluster size: " << std::setw(22) << m.meanClusterSize << " |\n";
    std::cout << "| Std dev:           " << std::setw(22) << m.stdDevClusterSize << " |\n";
    std::cout << "| CV (balance):      " << std::setw(22) << m.coefficientOfVariation << " |\n";
    std::cout << "| Imbalance:         " << std::setw(22) << m.imbalance << " |\n";
    std::cout << "+-------------------------------------------------------+\n";

    // Качество разбиения
    std::cout << "| Cut:               " << std::setw(22) << m.cut << " |\n";
    std::cout << "| Modularity (Q):    " << std::setw(22) << m.modularity << " |\n";
    std::cout << "| Internal density:  " << std::setw(22) << m.internalDensity << " |\n";
    std::cout << "+-------------------------------------------------------+\n";

    // Метрики параллелизма
    std::cout << "| * K (colors):      " << std::setw(22) << m.greedyColorsEstimate << " |\n";
    std::cout << "| Min colors (D+1):  " << std::setw(22) << m.minColorsEstimate << " |\n";
    std::cout << "| Parallelism (C/K): " << std::setw(22) << m.parallelismPotential << " |\n";
    std::cout << "+-------------------------------------------------------+\n";

    // GPU-метрики
    std::cout << "| Warp efficiency:   " << std::setw(22) << m.warpEfficiency << " |\n";
    std::cout << "| Load balance:      " << std::setw(22) << m.loadBalanceScore << " |\n";
    std::cout << "+-------------------------------------------------------+\n";
    std::cout << "| Cluster graph:                                        |\n";
    std::cout << "|   Max degree:      " << std::setw(22) << m.maxDegreeInClusterGraph << " |\n";
    std::cout << "|   Avg degree:      " << std::setw(22) << m.avgDegreeInClusterGraph << " |\n";
    std::cout << "+-------------------------------------------------------+\n";

    // Гистограмма размеров кластеров (если не слишком много)
    if (!m.clusterSizeHistogram.empty() && m.clusterSizeHistogram.size() <= 30) {
        std::cout << "\nCluster size histogram:\n";
        int maxCount = 0;
        for (const auto& [size, count] : m.clusterSizeHistogram) {
            maxCount = std::max(maxCount, count);
        }
        int maxBarLen = 40;
        for (const auto& [size, count] : m.clusterSizeHistogram) {
            int barLen = static_cast<int>(static_cast<double>(count) / maxCount * maxBarLen);
            if (barLen < 1 && count > 0) barLen = 1;
            std::cout << "  " << std::setw(4) << size << ": "
                      << std::string(barLen, '*') << " (" << count << ")\n";
        }
    }
}

int main(int argc, char* argv[]) {
    auto args = parseArgs(argc, argv);

    if (!args.verbose) {
        Logger::getInstance().setLevel(LogLevel::WARNING);
    } else {
        Logger::getInstance().setLevel(LogLevel::DEBUG);
    }

    LOG_INFO("=== Clustering Benchmark Started ===");

    Graph graph;
    std::string graphName;

    // ============================================================
    // ВЫБОР ИСТОЧНИКА ГРАФА
    // ============================================================
    if (args.generateGrid) {
        // --- Генерация сетки ---
        if (args.generate3D) {
            LOG_INFO("Generating 3D grid: " + std::to_string(args.gridSize) + "x" +
                     std::to_string(args.gridSize) + "x" + std::to_string(args.gridDepth));
            graph = GraphGenerator::generateGrid3D(args.gridSize, args.gridSize, args.gridDepth);
            graphName = "grid3d_" + std::to_string(args.gridSize) + "x" +
                        std::to_string(args.gridSize) + "x" + std::to_string(args.gridDepth);
        } else {
            LOG_INFO("Generating 2D grid: " + std::to_string(args.gridSize) + "x" +
                     std::to_string(args.gridSize));
            graph = GraphGenerator::generateGrid2D(args.gridSize, args.gridSize);
            graphName = "grid2d_" + std::to_string(args.gridSize) + "x" +
                        std::to_string(args.gridSize);
        }
    }
    else if (args.generateRandom) {
        // --- Генерация случайного графа ---
        LOG_INFO("Generating random graph: N=" + std::to_string(args.randomN) +
                 ", p=" + std::to_string(args.randomP));
        GraphGenerator gen;
        graph = gen.generateErdosRenyi(args.randomN, args.randomP);
        graphName = "random_" + std::to_string(args.randomN) + "_" +
                    std::to_string(args.randomP).substr(0, 4);
    }
    else if (!args.inputFile.empty()) {
        // --- Загрузка из файла ---
        LOG_INFO("Loading graph from: " + args.inputFile);
        try {
            graph = GraphLoader::loadFromCSV(args.inputFile);
            graphName = std::filesystem::path(args.inputFile).stem().string();
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to load graph: " + std::string(e.what()));
            return 1;
        }
    } else {
        LOG_ERROR("No input source specified. Use -g, -r, or -i");
        printUsage(argv[0]);
        return 1;
    }

    graph.printStats();

    LOG_INFO("Graph loaded: |V|=" + std::to_string(graph.getNumVertices()) +
             ", |E|=" + std::to_string(graph.getNumEdges()));

    ClusteringParams params;
    params.maxClusterSize = args.maxClusterSize;
    params.localOptimizationIters = args.localIters;
    params.enforceWarpMultiple = args.enforceWarpMultiple;

    LOG_INFO("Clustering params: " + params.toString());

    std::vector<ExperimentResult> allResults;

    for (const auto& algoName : args.algorithms) {
        LOG_INFO("Running algorithm: " + algoName);

        try {
            auto algorithm = createAlgorithm(algoName);
            ExperimentResult result;
            result.graphName = graphName;
            result.algorithmName = algoName;
            result.params = params;
            result.timestamp = Logger::getInstance().getTimestamp();

            Timer timer;
            timer.start();

            auto clusters = algorithm->computeClusters(graph, params);

            timer.stop();
            result.executionTimeMs = timer.getElapsedMs();
            result.metrics = MetricsCalculator::compute(graph, clusters);

            LOG_INFO("  Completed in " + std::to_string(result.executionTimeMs) + " ms");
            LOG_INFO("  Metrics: K=" + std::to_string(result.metrics.greedyColorsEstimate) +
                     ", cut=" + std::to_string(result.metrics.cut) +
                     ", CV=" + std::to_string(result.metrics.coefficientOfVariation) +
                     ", clusters=" + std::to_string(result.metrics.numClusters));

            allResults.push_back(result);

        } catch (const std::exception& e) {
            LOG_ERROR("Algorithm " + algoName + " failed: " + std::string(e.what()));
        }
    }

    if (!allResults.empty()) {
        std::filesystem::create_directories(std::filesystem::path(args.outputFile).parent_path());

        ResultExporter::exportMultipleToCSV(allResults, args.outputFile);
        ResultExporter::exportSummary(allResults, args.outputFile + ".summary");

        LOG_INFO("Results exported to: " + args.outputFile);

        // ============================================================
        // SUMMARY TABLE (ASCII, без псевдографики)
        // ============================================================
        std::cout << "\n";
        std::cout << "============================================================\n";
        std::cout << "                        SUMMARY                             \n";
        std::cout << "============================================================\n";
        std::cout << " Algorithm   |  K |   Cut   |   CV   |  C  |  T_c(ms)       \n";
        std::cout << "-------------|-----|---------|--------|-----|---------------\n";

        for (const auto& r : allResults) {
            std::cout << " " << std::left << std::setw(11) << r.algorithmName << " | "
                      << std::setw(3) << r.metrics.greedyColorsEstimate << " | "
                      << std::setw(7) << std::fixed << std::setprecision(0) << r.metrics.cut << " | "
                      << std::setw(6) << std::setprecision(3) << r.metrics.coefficientOfVariation << " | "
                      << std::setw(3) << r.metrics.numClusters << " | "
                      << std::setw(10) << std::setprecision(2) << r.executionTimeMs << "\n";
        }
        std::cout << "============================================================\n";

        // ============================================================
        // ДЕТАЛЬНЫЕ МЕТРИКИ (только в verbose режиме)
        // ============================================================
        if (args.verbose) {
            for (const auto& r : allResults) {
                std::cout << "\n--- " << r.algorithmName << " ---";
                printDetailedMetrics(r);
            }
        }
    }

    LOG_INFO("=== Clustering Benchmark Finished ===");
    return 0;
}