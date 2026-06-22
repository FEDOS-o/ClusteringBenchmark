#include "metrics/MetricsCalculator.hpp"
#include <algorithm>
#include <set>
#include <queue>
#include <cmath>
#include <map>
#include <numeric>

namespace clustering {

ClusteringMetrics MetricsCalculator::compute(
    const Graph& graph,
    const ClusteringResult& clusters
) {
    ClusteringMetrics metrics;

    // === 1. Базовые метрики ===
    metrics.numClusters = static_cast<int>(clusters.size());
    metrics.numVertices = graph.getNumVertices();
    metrics.numEdges = graph.getNumEdges();

    // === 2. Размеры кластеров ===
    metrics = computeClusterSizeMetrics(clusters);
    metrics.numClusters = static_cast<int>(clusters.size());
    metrics.numVertices = graph.getNumVertices();
    metrics.numEdges = graph.getNumEdges();

    // === 3. Качество разбиения ===
    metrics.cut = computeCut(graph, clusters);
    metrics.modularity = computeModularity(graph, clusters);
    metrics.internalDensity = computeInternalDensity(graph, clusters);

    // === 4. Граф кластеров ===
    Graph clusterGraph = buildClusterGraph(graph, clusters);
    metrics.maxDegreeInClusterGraph = computeMaxDegree(clusterGraph);
    metrics.avgDegreeInClusterGraph = computeAvgDegree(clusterGraph);
    metrics.minColorsEstimate = metrics.maxDegreeInClusterGraph + 1;
    metrics.greedyColorsEstimate = estimateGreedyColors(clusterGraph);

    // === 5. Параллелизм ===
    metrics.parallelismPotential = static_cast<double>(metrics.numClusters) /
                                   static_cast<double>(metrics.greedyColorsEstimate);

    // === 6. GPU-специфичные метрики ===
    metrics.warpEfficiency = computeWarpEfficiency(clusters);
    metrics.loadBalanceScore = computeLoadBalanceScore(clusters);

    // === 7. Гистограмма ===
    for (const auto& cluster : clusters) {
        metrics.clusterSizeHistogram[static_cast<int>(cluster.size())]++;
    }

    return metrics;
}

// === Реализация отдельных метрик ===

ClusteringMetrics MetricsCalculator::computeClusterSizeMetrics(const ClusteringResult& clusters) {
    ClusteringMetrics metrics;

    if (clusters.empty()) {
        metrics.minClusterSize = 0;
        metrics.maxClusterSize = 0;
        metrics.meanClusterSize = 0;
        metrics.stdDevClusterSize = 0;
        metrics.coefficientOfVariation = 0;
        metrics.imbalance = 0;
        return metrics;
    }

    std::vector<double> sizes;
    sizes.reserve(clusters.size());

    for (const auto& cluster : clusters) {
        sizes.push_back(static_cast<double>(cluster.size()));
    }

    metrics.minClusterSize = *std::min_element(sizes.begin(), sizes.end());
    metrics.maxClusterSize = *std::max_element(sizes.begin(), sizes.end());

    double sum = std::accumulate(sizes.begin(), sizes.end(), 0.0);
    metrics.meanClusterSize = sum / sizes.size();

    double sq_sum = std::inner_product(sizes.begin(), sizes.end(), sizes.begin(), 0.0);
    double variance = (sq_sum / sizes.size()) - (metrics.meanClusterSize * metrics.meanClusterSize);
    metrics.stdDevClusterSize = std::sqrt(std::max(0.0, variance));

    metrics.coefficientOfVariation = metrics.stdDevClusterSize / metrics.meanClusterSize;
    metrics.imbalance = (metrics.maxClusterSize / metrics.meanClusterSize) - 1.0;

    return metrics;
}

double MetricsCalculator::computeCut(
    const Graph& graph,
    const ClusteringResult& clusters
) {
    if (clusters.empty()) return 0.0;

    // Построение карты вершина -> кластер
    std::vector<int> vertexToCluster(graph.getNumVertices(), -1);
    for (const auto& cluster : clusters) {
        for (VertexId v : cluster.vertices) {
            vertexToCluster[v] = cluster.id;
        }
    }

    double cut = 0.0;

    // Проход по всем рёбрам
    for (VertexId u = 0; u < graph.getNumVertices(); ++u) {
        const auto& neighbors = graph.getNeighbors(u);
        for (VertexId v : neighbors) {
            if (u < v) {  // каждое ребро один раз
                if (vertexToCluster[u] != vertexToCluster[v]) {
                    cut += 1.0;
                }
            }
        }
    }

    return cut;
}

double MetricsCalculator::computeModularity(
    const Graph& graph,
    const ClusteringResult& clusters
) {
    if (clusters.empty()) return 0.0;

    std::vector<int> vertexToCluster(graph.getNumVertices(), -1);
    for (const auto& cluster : clusters) {
        for (VertexId v : cluster.vertices) {
            vertexToCluster[v] = cluster.id;
        }
    }

    double m = static_cast<double>(graph.getNumEdges());
    if (m == 0) return 0.0;

    double Q = 0.0;

    // Считаем внутренние рёбра и степени сообществ
    std::map<int, double> communityInternal;
    std::map<int, double> communityDegree;

    for (VertexId u = 0; u < graph.getNumVertices(); ++u) {
        int cu = vertexToCluster[u];
        if (cu < 0) continue;

        communityDegree[cu] += static_cast<double>(graph.getDegree(u));

        const auto& neighbors = graph.getNeighbors(u);
        for (VertexId v : neighbors) {
            if (u < v && vertexToCluster[u] == vertexToCluster[v]) {
                communityInternal[cu] += 1.0;
            }
        }
    }

    for (const auto& [c, internal] : communityInternal) {
        double deg = communityDegree[c];
        Q += internal / m - (deg / (2.0 * m)) * (deg / (2.0 * m));
    }

    return Q;
}

double MetricsCalculator::computeCoefficientOfVariation(const ClusteringResult& clusters) {
    if (clusters.empty()) return 0.0;

    double sum = 0.0;
    double sumSq = 0.0;
    for (const auto& c : clusters) {
        double sz = static_cast<double>(c.size());
        sum += sz;
        sumSq += sz * sz;
    }
    double mean = sum / clusters.size();
    double variance = (sumSq / clusters.size()) - (mean * mean);
    double stddev = std::sqrt(std::max(0.0, variance));
    return stddev / mean;
}

double MetricsCalculator::computeImbalance(const ClusteringResult& clusters) {
    if (clusters.empty()) return 0.0;

    double sum = 0.0;
    double maxSz = 0.0;
    for (const auto& c : clusters) {
        double sz = static_cast<double>(c.size());
        sum += sz;
        maxSz = std::max(maxSz, sz);
    }
    double mean = sum / clusters.size();
    return (maxSz / mean) - 1.0;
}

double MetricsCalculator::computeWarpEfficiency(const ClusteringResult& clusters) {
    if (clusters.empty()) return 0.0;

    // Насколько размеры кластеров кратны 32
    double totalEfficiency = 0.0;
    for (const auto& cluster : clusters) {
        size_t size = cluster.size();
        size_t warpCount = size / 32;
        size_t remainder = size % 32;

        double efficiency = 1.0;
        if (remainder != 0) {
            // Если не кратно 32, последний warp загружен не полностью
            efficiency = static_cast<double>(warpCount * 32 + remainder) /
                        static_cast<double>((warpCount + 1) * 32);
        }
        totalEfficiency += efficiency;
    }

    return totalEfficiency / clusters.size();
}

double MetricsCalculator::computeLoadBalanceScore(const ClusteringResult& clusters) {
    if (clusters.empty()) return 0.0;

    double cv = computeCoefficientOfVariation(clusters);
    // Чем меньше CV, тем лучше баланс
    return 1.0 / (1.0 + cv);
}

double MetricsCalculator::computeInternalDensity(
    const Graph& graph,
    const ClusteringResult& clusters
) {
    if (clusters.empty()) return 0.0;

    std::vector<int> vertexToCluster(graph.getNumVertices(), -1);
    for (const auto& cluster : clusters) {
        for (VertexId v : cluster.vertices) {
            vertexToCluster[v] = cluster.id;
        }
    }

    double internalEdges = 0.0;
    double totalPossibleInternal = 0.0;

    for (VertexId u = 0; u < graph.getNumVertices(); ++u) {
        const auto& neighbors = graph.getNeighbors(u);
        for (VertexId v : neighbors) {
            if (u < v && vertexToCluster[u] == vertexToCluster[v]) {
                internalEdges += 1.0;
            }
        }
    }

    // Считаем максимально возможное число внутренних рёбер
    for (const auto& cluster : clusters) {
        size_t sz = cluster.size();
        totalPossibleInternal += sz * (sz - 1) / 2.0;
    }

    return (totalPossibleInternal > 0) ? internalEdges / totalPossibleInternal : 0.0;
}

Graph MetricsCalculator::buildClusterGraph(
    const Graph& original,
    const ClusteringResult& clusters
) {
    // Карта вершина -> кластер
    std::vector<int> vertexToCluster(original.getNumVertices(), -1);
    for (const auto& cluster : clusters) {
        for (VertexId v : cluster.vertices) {
            vertexToCluster[v] = cluster.id;
        }
    }

    int numClusters = static_cast<int>(clusters.size());

    // Собираем рёбра между кластерами (используем set для уникальности)
    std::set<std::pair<int, int>> clusterEdges;

    for (VertexId u = 0; u < original.getNumVertices(); ++u) {
        const auto& neighbors = original.getNeighbors(u);
        for (VertexId v : neighbors) {
            if (u < v) {
                int cu = vertexToCluster[u];
                int cv = vertexToCluster[v];
                if (cu != cv && cu != -1 && cv != -1) {
                    if (cu > cv) std::swap(cu, cv);
                    clusterEdges.insert({cu, cv});
                }
            }
        }
    }

    EdgeList edges;
    for (const auto& e : clusterEdges) {
        edges.push_back({static_cast<VertexId>(e.first),
                         static_cast<VertexId>(e.second), 1.0f});
    }

    Graph clusterGraph;
    clusterGraph.buildFromEdges(numClusters, edges);

    return clusterGraph;
}

int MetricsCalculator::estimateGreedyColors(const Graph& clusterGraph) {
    VertexId n = clusterGraph.getNumVertices();
    if (n == 0) return 0;

    std::vector<int> colors(n, -1);

    // Сортировка по степени (убывание) — лучшая эвристика
    std::vector<VertexId> order(n);
    for (VertexId v = 0; v < n; ++v) order[v] = v;
    std::sort(order.begin(), order.end(), [&](VertexId a, VertexId b) {
        return clusterGraph.getDegree(a) > clusterGraph.getDegree(b);
    });

    for (VertexId v : order) {
        std::vector<bool> used(n, false);

        const auto& neighbors = clusterGraph.getNeighbors(v);
        for (VertexId u : neighbors) {
            if (colors[u] != -1) {
                used[colors[u]] = true;
            }
        }

        int color = 0;
        while (color < static_cast<int>(used.size()) && used[color]) {
            color++;
        }
        colors[v] = color;
    }

    int maxColor = 0;
    for (int c : colors) {
        maxColor = std::max(maxColor, c);
    }

    return maxColor + 1;
}

int MetricsCalculator::computeMaxDegree(const Graph& clusterGraph) {
    VertexId n = clusterGraph.getNumVertices();
    if (n == 0) return 0;

    int maxDeg = 0;
    for (VertexId v = 0; v < n; ++v) {
        maxDeg = std::max(maxDeg, static_cast<int>(clusterGraph.getDegree(v)));
    }
    return maxDeg;
}

double MetricsCalculator::computeAvgDegree(const Graph& clusterGraph) {
    VertexId n = clusterGraph.getNumVertices();
    if (n == 0) return 0.0;

    double sum = 0.0;
    for (VertexId v = 0; v < n; ++v) {
        sum += static_cast<double>(clusterGraph.getDegree(v));
    }
    return sum / n;
}

} // namespace clustering