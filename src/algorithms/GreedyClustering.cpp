#include "algorithms/GreedyClustering.hpp"
#include "utils/Timer.hpp"
#include <algorithm>
#include <queue>
#include <vector>

namespace clustering {

ClusteringResult GreedyClustering::computeClusters(
    const Graph& graph,
    const ClusteringParams& params
) {
    Timer timer;
    timer.start();

    VertexId n = graph.getNumVertices();
    std::vector<bool> visited(n, false);
    ClusteringResult clusters;
    int nextClusterId = 0;

    // Сортировка вершин по степени (убывание) для лучшего качества
    std::vector<VertexId> order(n);
    for (VertexId v = 0; v < n; ++v) order[v] = v;

    std::sort(order.begin(), order.end(), [&](VertexId a, VertexId b) {
        return graph.getDegree(a) > graph.getDegree(b);
    });

    for (VertexId start : order) {
        if (visited[start]) continue;

        // BFS для формирования кластера
        std::vector<VertexId> cluster;
        std::queue<VertexId> q;
        q.push(start);
        visited[start] = true;
        cluster.push_back(start);

        while (!q.empty() && static_cast<int>(cluster.size()) < params.maxClusterSize) {
            VertexId v = q.front();
            q.pop();

            const auto& neighbors = graph.getNeighbors(v);
            for (VertexId u : neighbors) {
                if (!visited[u] && static_cast<int>(cluster.size()) < params.maxClusterSize) {
                    visited[u] = true;
                    q.push(u);
                    cluster.push_back(u);
                }
            }
        }

        // Пост-обработка: если требуется кратность 32
        if (params.enforceWarpMultiple && cluster.size() % 32 != 0) {
            // Обрезаем до ближайшего кратного 32 (снизу)
            size_t newSize = (cluster.size() / 32) * 32;
            if (newSize == 0 && cluster.size() > 0) newSize = 32;
            cluster.resize(newSize);
        }

        if (!cluster.empty()) {
            Cluster c;
            c.id = nextClusterId++;
            c.vertices = std::move(cluster);
            clusters.push_back(c);
        }
    }

    timer.stop();
    setLastExecutionTime(timer.getElapsedMs());

    return clusters;
}

} // namespace clustering