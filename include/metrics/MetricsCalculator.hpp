#pragma once
#include "ClusteringMetrics.hpp"
#include "../core/Graph.hpp"
#include "../core/Cluster.hpp"

namespace clustering {

    class MetricsCalculator {
    public:
        static ClusteringMetrics compute(
            const Graph& graph,
            const ClusteringResult& clusters
        );

        // Отдельные метрики (для отладки/тестов)
        static double computeCut(const Graph& graph, const ClusteringResult& clusters);
        static double computeModularity(const Graph& graph, const ClusteringResult& clusters);
        static double computeCoefficientOfVariation(const ClusteringResult& clusters);
        static double computeImbalance(const ClusteringResult& clusters);
        static int estimateGreedyColors(const Graph& clusterGraph);

    private:
        static Graph buildClusterGraph(const Graph& original, const ClusteringResult& clusters);
        static double computeClusterInternalDensity(const Graph& graph, const Cluster& cluster);
    };

} // namespace clustering