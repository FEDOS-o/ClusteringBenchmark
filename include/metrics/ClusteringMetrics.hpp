#pragma once
#include "../core/Types.hpp"
#include "../core/Cluster.hpp"
#include <string>
#include <vector>
#include <map>

namespace clustering {

    struct ClusteringMetrics {
        // Базовые
        int numClusters = 0;
        int numVertices = 0;
        int numEdges = 0;

        // Размеры кластеров
        double minClusterSize = 0;
        double maxClusterSize = 0;
        double meanClusterSize = 0;
        double stdDevClusterSize = 0;
        double coefficientOfVariation = 0;  // CV = σ/μ
        double imbalance = 0;                // max/μ - 1

        // Качество разбиения
        double cut = 0;
        double modularity = 0;

        // Метрики для параллельного выполнения
        int minColorsEstimate = 0;      // Δ+1
        int greedyColorsEstimate = 0;   // жадная раскраска

        // Гистограммы
        std::map<int, int> clusterSizeHistogram;

        std::string toCSVHeader() const;
        std::string toCSVRow() const;
        void print() const;
    };

} // namespace clustering