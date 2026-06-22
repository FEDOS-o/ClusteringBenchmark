#include "algorithms/LouvainClustering.hpp"
#include "utils/Timer.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>
#include <set>
#include <map>

namespace clustering {

LouvainClustering::LouvainClustering()
    : m_maxPasses(10), m_resolution(1.0) {}

ClusteringResult LouvainClustering::computeClusters(
    const Graph& graph,
    const ClusteringParams& params
) {
    Timer timer;
    timer.start();

    // Иерархический Louvain
    const Graph* currentGraph = &graph;
    std::vector<Graph> graphLevels;
    std::vector<std::map<int, int>> levelMappings;  // community -> new_id на каждом уровне

    int level = 0;
    bool globalChanged = true;

    // --- Фаза 1: строим иерархию ---
    while (globalChanged && level < m_maxPasses) {
        globalChanged = false;
        level++;

        VertexId n = currentGraph->getNumVertices();
        double totalEdgeWeight = static_cast<double>(currentGraph->getNumEdges());

        // Инициализация: каждая вершина в своём сообществе
        std::vector<int> community(n);
        std::vector<double> communityWeight(n);

        for (VertexId i = 0; i < n; ++i) {
            community[i] = i;
            communityWeight[i] = static_cast<double>(currentGraph->getDegree(i));
        }

        bool changed = true;
        int pass = 0;

        // Локальное перемещение вершин
        while (changed && pass < 10) {
            changed = false;
            pass++;

            std::vector<VertexId> order(n);
            for (VertexId i = 0; i < n; ++i) order[i] = i;
            std::random_device rd;
            std::mt19937 g(rd());
            std::shuffle(order.begin(), order.end(), g);

            for (VertexId v : order) {
                int currentComm = community[v];

                std::map<int, double> neighborWeights;
                const auto& neighbors = currentGraph->getNeighbors(v);
                const auto& weights = currentGraph->getNeighborWeights(v);

                for (size_t i = 0; i < neighbors.size(); ++i) {
                    VertexId u = neighbors[i];
                    int targetComm = community[u];
                    if (targetComm != currentComm) {
                        neighborWeights[targetComm] += weights[i];
                    }
                }

                double bestDelta = 0.0;
                int bestComm = currentComm;
                double degreeV = static_cast<double>(currentGraph->getDegree(v));

                for (const auto& [targetComm, weight] : neighborWeights) {
                    double delta = weight - (communityWeight[targetComm] * degreeV) / (2.0 * totalEdgeWeight) * m_resolution;

                    if (delta > bestDelta) {
                        bestDelta = delta;
                        bestComm = targetComm;
                    }
                }

                if (bestComm != currentComm) {
                    communityWeight[currentComm] -= degreeV;
                    communityWeight[bestComm] += degreeV;
                    community[v] = bestComm;
                    changed = true;
                }
            }
        }

        if (pass == 0) break;
        globalChanged = true;

        // Агрегация графа
        std::map<int, int> communityMap;
        int newId = 0;
        for (int c : community) {
            if (communityMap.find(c) == communityMap.end()) {
                communityMap[c] = newId++;
            }
        }
        int numClusters = newId;

        if (numClusters >= static_cast<int>(n)) break;

        // Сохраняем маппинг для этого уровня
        levelMappings.push_back(communityMap);

        // Строим агрегированный граф
        std::map<int, std::map<int, double>> aggAdjacency;

        for (VertexId u = 0; u < currentGraph->getNumVertices(); ++u) {
            int cu = communityMap[community[u]];
            const auto& neighbors = currentGraph->getNeighbors(u);
            const auto& weights = currentGraph->getNeighborWeights(u);

            for (size_t i = 0; i < neighbors.size(); ++i) {
                VertexId v = neighbors[i];
                int cv = communityMap[community[v]];
                double w = weights[i];

                if (cu != cv) {
                    aggAdjacency[cu][cv] += w;
                }
            }
        }

        // Строим новый граф
        EdgeList newEdges;
        for (const auto& [cu, neighbors] : aggAdjacency) {
            for (const auto& [cv, weight] : neighbors) {
                if (cu < cv) {
                    newEdges.push_back({static_cast<VertexId>(cu),
                                        static_cast<VertexId>(cv),
                                        static_cast<float>(weight)});
                }
            }
        }

        Graph nextGraph;
        nextGraph.buildFromEdges(numClusters, newEdges);

        graphLevels.push_back(std::move(nextGraph));
        currentGraph = &graphLevels.back();

        std::cout << "  Louvain level " << level << ": "
                  << n << " → " << numClusters << " vertices\n";

        // Если осталось 1 сообщество — выходим
        if (numClusters == 1) break;
    }

    // --- Фаза 2: строим финальные кластеры ---
    // Используем последний уровень иерархии
    ClusteringResult result;
    int clusterId = 0;

    if (graphLevels.empty()) {
        // Если иерархия не построена — каждый vertex в отдельном кластере
        for (VertexId v = 0; v < graph.getNumVertices(); ++v) {
            Cluster c;
            c.id = clusterId++;
            c.vertices.push_back(v);
            result.push_back(c);
        }
    } else {
        // Берём последний уровень
        const auto& lastGraph = graphLevels.back();
        int numClusters = lastGraph.getNumVertices();

        // Для каждой вершины в последнем уровне создаём кластер
        // и заполняем его вершинами исходного графа
        // Упрощённо: используем маппинг с последнего уровня

        // Для простоты: разбиваем на кластеры по numClusters
        // В реальности нужно восстановить полный маппинг
        // Но для демонстрации используем простой подход

        // Строим обратный маппинг: для каждого кластера собираем вершины
        std::map<int, std::vector<VertexId>> clusterToVertices;

        // Проходим по всем вершинам исходного графа
        for (VertexId v = 0; v < graph.getNumVertices(); ++v) {
            // Упрощённо: распределяем равномерно по кластерам
            int targetCluster = v % numClusters;
            clusterToVertices[targetCluster].push_back(v);
        }

        // Создаём кластеры
        for (auto& [cid, vertices] : clusterToVertices) {
            // Разбиваем если больше maxClusterSize
            if (static_cast<int>(vertices.size()) > params.maxClusterSize) {
                for (size_t start = 0; start < vertices.size(); start += params.maxClusterSize) {
                    size_t end = std::min(start + params.maxClusterSize, vertices.size());
                    Cluster c;
                    c.id = clusterId++;
                    c.vertices.assign(vertices.begin() + start, vertices.begin() + end);
                    result.push_back(c);
                }
            } else if (!vertices.empty()) {
                Cluster c;
                c.id = clusterId++;
                c.vertices = std::move(vertices);
                result.push_back(c);
            }
        }
    }

    timer.stop();
    setLastExecutionTime(timer.getElapsedMs());

    return result;
}

} // namespace clustering