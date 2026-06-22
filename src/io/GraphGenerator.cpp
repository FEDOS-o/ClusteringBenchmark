#include "io/GraphGenerator.hpp"
#include <cmath>

namespace clustering {

GraphGenerator::GraphGenerator(uint64_t seed) : m_rng(seed) {}

Graph GraphGenerator::generateGrid2D(int width, int height) {
    EdgeList edges;
    
    auto vertexId = [width](int x, int y) -> VertexId {
        return y * width + x;
    };
    
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            VertexId v = vertexId(x, y);
            
            // Право
            if (x + 1 < width) {
                edges.push_back({v, vertexId(x + 1, y), 1.0f});
            }
            // Низ
            if (y + 1 < height) {
                edges.push_back({v, vertexId(x, y + 1), 1.0f});
            }
        }
    }
    
    Graph graph;
    graph.buildFromEdges(width * height, edges);
    return graph;
}

Graph GraphGenerator::generateGrid3D(int width, int height, int depth) {
    EdgeList edges;
    
    auto vertexId = [width, height](int x, int y, int z) -> VertexId {
        return (z * height + y) * width + x;
    };
    
    for (int z = 0; z < depth; ++z) {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                VertexId v = vertexId(x, y, z);
                
                if (x + 1 < width)
                    edges.push_back({v, vertexId(x + 1, y, z), 1.0f});
                if (y + 1 < height)
                    edges.push_back({v, vertexId(x, y + 1, z), 1.0f});
                if (z + 1 < depth)
                    edges.push_back({v, vertexId(x, y, z + 1), 1.0f});
            }
        }
    }
    
    Graph graph;
    graph.buildFromEdges(width * height * depth, edges);
    return graph;
}

Graph GraphGenerator::generateErdosRenyi(VertexId n, double p) {
    EdgeList edges;
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    
    for (VertexId u = 0; u < n; ++u) {
        for (VertexId v = u + 1; v < n; ++v) {
            if (dist(m_rng) < p) {
                edges.push_back({u, v, 1.0f});
            }
        }
    }
    
    Graph graph;
    graph.buildFromEdges(n, edges);
    return graph;
}

Graph GraphGenerator::generateSmallWorld(VertexId n, int k, double beta) {
    EdgeList edges;
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    
    // Кольцо с k ближайшими соседями
    for (VertexId i = 0; i < n; ++i) {
        for (int j = 1; j <= k / 2; ++j) {
            VertexId v = (i + j) % n;
            
            // С вероятностью beta перенаправляем ребро
            if (dist(m_rng) < beta) {
                VertexId target = static_cast<VertexId>(dist(m_rng) * n);
                if (target != i) {
                    edges.push_back({i, target, 1.0f});
                }
            } else {
                edges.push_back({i, v, 1.0f});
            }
        }
    }
    
    Graph graph;
    graph.buildFromEdges(n, edges);
    return graph;
}

Graph GraphGenerator::generateChain(VertexId n) {
    EdgeList edges;
    for (VertexId i = 0; i < n - 1; ++i) {
        edges.push_back({i, i + 1, 1.0f});
    }
    
    Graph graph;
    graph.buildFromEdges(n, edges);
    return graph;
}

Graph GraphGenerator::generateComplete(VertexId n) {
    EdgeList edges;
    for (VertexId u = 0; u < n; ++u) {
        for (VertexId v = u + 1; v < n; ++v) {
            edges.push_back({u, v, 1.0f});
        }
    }
    
    Graph graph;
    graph.buildFromEdges(n, edges);
    return graph;
}

} // namespace clustering