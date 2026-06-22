#pragma once
#include "ClusteringMetrics.hpp"
#include "../core/Graph.hpp"
#include "../core/Cluster.hpp"

namespace clustering {

    class MetricsCalculator {
    public:
        // Основной метод: вычисление ВСЕХ метрик
        static ClusteringMetrics compute(
            const Graph& graph,
            const ClusteringResult& clusters
        );

        // === Отдельные метрики (для отладки/тестов) ===
        static double computeCut(const Graph& graph, const ClusteringResult& clusters);
        static double computeModularity(const Graph& graph, const ClusteringResult& clusters);
        static double computeCoefficientOfVariation(const ClusteringResult& clusters);
        static double computeImbalance(const ClusteringResult& clusters);
        static double computeWarpEfficiency(const ClusteringResult& clusters);
        static double computeLoadBalanceScore(const ClusteringResult& clusters);
        static double computeInternalDensity(const Graph& graph, const ClusteringResult& clusters);

        // === Метрики для графа кластеров ===
        static Graph buildClusterGraph(const Graph& original, const ClusteringResult& clusters);
        static int estimateGreedyColors(const Graph& clusterGraph);
        static int computeMaxDegree(const Graph& clusterGraph);
        static double computeAvgDegree(const Graph& clusterGraph);

    private:
        static ClusteringMetrics computeClusterSizeMetrics(const ClusteringResult& clusters);
        static ClusteringMetrics computeParallelismMetrics(const Graph& clusterGraph, int numClusters);
    };

} // namespace clustering