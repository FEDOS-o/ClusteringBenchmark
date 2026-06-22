#include "algorithms/GLBClustering.hpp"
#include "utils/Timer.hpp"
#include <algorithm>
#include <queue>
#include <map>
#include <cmath>
#include <iostream>

namespace clustering {

ClusteringResult GLBClustering::computeClusters(
    const Graph& graph,
    const ClusteringParams& params
) {
    Timer timer;
    timer.start();

    // Фаза 1: жадная инициализация
    ClusteringResult clusters = greedyInitialization(graph, params.maxClusterSize);

    // Если кластеров нет — возвращаем пустой результат
    if (clusters.empty()) {
        timer.stop();
        setLastExecutionTime(timer.getElapsedMs());
        return clusters;
    }

    // Фаза 2: локальная оптимизация
    if (params.localOptimizationIters > 0) {
        localOptimization(graph, clusters, params.localOptimizationIters, params.qualityTolerance);
    }

    // Фаза 3: обеспечение кратности 32
    if (params.enforceWarpMultiple) {
        enforceWarpMultiple(clusters);
    }

    timer.stop();
    setLastExecutionTime(timer.getElapsedMs());

    return clusters;
}

ClusteringResult GLBClustering::updateClusters(
    const Graph& graph,
    const ClusteringResult& /*previous*/,
    const ClusteringParams& params
) {
    return computeClusters(graph, params);
}

ClusteringResult GLBClustering::greedyInitialization(const Graph& graph, int maxSize) {
    VertexId n = graph.getNumVertices();
    std::vector<bool> visited(n, false);
    ClusteringResult clusters;
    int nextClusterId = 0;

    // Сортировка по степени (убывание)
    std::vector<VertexId> order(n);
    for (VertexId v = 0; v < n; ++v) order[v] = v;

    std::sort(order.begin(), order.end(), [&](VertexId a, VertexId b) {
        return graph.getDegree(a) > graph.getDegree(b);
    });

    for (VertexId start : order) {
        if (visited[start]) continue;

        std::vector<VertexId> cluster;
        std::queue<VertexId> q;
        q.push(start);
        visited[start] = true;
        cluster.push_back(start);

        while (!q.empty() && static_cast<int>(cluster.size()) < maxSize) {
            VertexId v = q.front();
            q.pop();

            const auto& neighbors = graph.getNeighbors(v);
            for (VertexId u : neighbors) {
                if (!visited[u] && static_cast<int>(cluster.size()) < maxSize) {
                    visited[u] = true;
                    q.push(u);
                    cluster.push_back(u);
                }
            }
        }

        if (!cluster.empty()) {
            Cluster c;
            c.id = nextClusterId++;
            c.vertices = std::move(cluster);
            clusters.push_back(c);
        }
    }

    return clusters;
}

void GLBClustering::localOptimization(
    const Graph& graph,
    ClusteringResult& clusters,
    int maxIterations,
    float tolerance
) {
    if (clusters.empty() || maxIterations <= 0) return;

    // Построение карты вершина -> кластер
    std::vector<int> vertexToCluster(graph.getNumVertices(), -1);
    for (const auto& cluster : clusters) {
        for (VertexId v : cluster.vertices) {
            vertexToCluster[v] = cluster.id;
        }
    }

    bool improved = true;
    int iter = 0;

    while (improved && iter < maxIterations) {
        improved = false;
        iter++;

        // Проход по всем вершинам
        for (VertexId v = 0; v < graph.getNumVertices(); ++v) {
            int currentCid = vertexToCluster[v];
            if (currentCid < 0 || currentCid >= static_cast<int>(clusters.size())) continue;

            // Если кластер пустой или состоит из 1 вершины — пропускаем
            if (clusters[currentCid].vertices.size() <= 1) continue;

            // Оценка перемещения
            MoveGain best = evaluateMove(graph, clusters, vertexToCluster, v, currentCid);

            if (best.totalScore > tolerance && best.toCluster != currentCid) {
                // Применяем перемещение
                applyMove(clusters, vertexToCluster, best);
                improved = true;
            }
        }
    }
}

GLBClustering::MoveGain GLBClustering::evaluateMove(
    const Graph& graph,
    const ClusteringResult& clusters,
    const std::vector<int>& vertexToCluster,
    VertexId vertex,
    int currentClusterId
) {
    MoveGain result;
    result.vertex = vertex;
    result.fromCluster = currentClusterId;
    result.toCluster = currentClusterId;
    result.cutDelta = 0.0f;
    result.balanceImprovement = 0.0f;
    result.totalScore = 0.0f;

    // Проверка корректности
    if (currentClusterId < 0 || currentClusterId >= static_cast<int>(clusters.size())) {
        return result;
    }

    // Сбор статистики по соседним кластерам
    std::map<int, int> neighborClusterCounts;
    const auto& neighbors = graph.getNeighbors(vertex);

    for (VertexId u : neighbors) {
        int targetCid = vertexToCluster[u];
        if (targetCid >= 0 && targetCid != currentClusterId) {
            neighborClusterCounts[targetCid]++;
        }
    }

    if (neighborClusterCounts.empty()) {
        return result;
    }

    // Текущие размеры кластеров
    size_t currentSize = clusters[currentClusterId].vertices.size();
    if (currentSize <= 1) return result;  // Нельзя переместить последнюю вершину

    // Вычисляем выгоду для каждого возможного кластера
    float bestDelta = 0.0f;
    int bestTarget = currentClusterId;

    for (const auto& [targetClusterId, count] : neighborClusterCounts) {
        if (targetClusterId == currentClusterId) continue;
        if (targetClusterId < 0 || targetClusterId >= static_cast<int>(clusters.size())) continue;

        size_t targetSize = clusters[targetClusterId].vertices.size();

        // Улучшение разреза
        float cutImprovement = static_cast<float>(count);

        // Выравнивание размеров (цель — 64, но не жёстко)
        float targetOptimal = 64.0f;
        float currentImbalance = std::abs(static_cast<float>(currentSize) - targetOptimal) / (targetOptimal + 1.0f);
        float newCurrentSize = static_cast<float>(currentSize - 1);
        float newTargetSize = static_cast<float>(targetSize + 1);
        float newImbalance = (std::abs(newCurrentSize - targetOptimal) + std::abs(newTargetSize - targetOptimal)) / (2.0f * targetOptimal + 1.0f);

        float balanceImprovement = currentImbalance - newImbalance;

        // Итоговая оценка
        float total = cutImprovement * 0.7f + balanceImprovement * 0.3f;

        if (total > bestDelta) {
            bestDelta = total;
            bestTarget = targetClusterId;
        }
    }

    if (bestTarget != currentClusterId) {
        result.toCluster = bestTarget;
        result.cutDelta = bestDelta;
        result.balanceImprovement = bestDelta;
        result.totalScore = bestDelta;
    }

    return result;
}

void GLBClustering::applyMove(
    ClusteringResult& clusters,
    std::vector<int>& vertexToCluster,
    const MoveGain& move
) {
    // Проверка корректности
    if (move.fromCluster < 0 || move.fromCluster >= static_cast<int>(clusters.size()) ||
        move.toCluster < 0 || move.toCluster >= static_cast<int>(clusters.size())) {
        return;
    }

    // Удаляем из исходного кластера
    auto& fromCluster = clusters[move.fromCluster];
    auto it = std::find(fromCluster.vertices.begin(), fromCluster.vertices.end(), move.vertex);
    if (it != fromCluster.vertices.end()) {
        fromCluster.vertices.erase(it);
    }

    // Добавляем в целевой кластер
    clusters[move.toCluster].vertices.push_back(move.vertex);

    // Обновляем карту
    vertexToCluster[move.vertex] = move.toCluster;

    // Если кластер стал пустым — удаляем его
    if (fromCluster.vertices.empty()) {
        // Обновляем vertexToCluster для всех, кто был в удалённом кластере
        // (их быть не должно, т.к. кластер пустой)
        clusters.erase(clusters.begin() + move.fromCluster);

        // Обновляем индексы в vertexToCluster
        for (VertexId v = 0; v < static_cast<VertexId>(vertexToCluster.size()); ++v) {
            if (vertexToCluster[v] > move.fromCluster) {
                vertexToCluster[v]--;
            }
        }
    }
}

void GLBClustering::enforceWarpMultiple(ClusteringResult& clusters) {
    if (clusters.empty()) return;

    for (auto& cluster : clusters) {
        size_t remainder = cluster.vertices.size() % 32;
        if (remainder != 0) {
            size_t newSize = (cluster.vertices.size() / 32) * 32;
            if (newSize == 0 && cluster.vertices.size() > 0) {
                newSize = 32;
            }
            if (newSize > 0) {
                cluster.vertices.resize(newSize);
            }
        }
    }

    // Удаляем пустые кластеры
    clusters.erase(
        std::remove_if(clusters.begin(), clusters.end(),
            [](const Cluster& c) { return c.vertices.empty(); }),
        clusters.end()
    );
}

} // namespace clustering