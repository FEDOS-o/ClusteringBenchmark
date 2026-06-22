#include "core/Graph.hpp"
#include <algorithm>
#include <stdexcept>
#include <iostream>

namespace clustering {

Graph::Graph(VertexId numVertices) {
    m_numVertices = numVertices;
    m_adjacency.resize(numVertices);
    m_weights.resize(numVertices);
    m_numEdges = 0;
}

Graph::Graph(VertexId numVertices, const EdgeList& edges) {
    buildFromEdges(numVertices, edges);
}

void Graph::buildFromEdges(VertexId numVertices, const EdgeList& edges) {
    m_numVertices = numVertices;
    m_adjacency.clear();
    m_weights.clear();
    m_adjacency.resize(numVertices);
    m_weights.resize(numVertices);
    m_numEdges = 0;

    for (const auto& e : edges) {
        addEdge(e.u, e.v, e.weight);
    }
}

void Graph::addVertex() {
    m_adjacency.emplace_back();
    m_weights.emplace_back();
    m_numVertices++;
}

void Graph::addEdge(VertexId u, VertexId v, Weight w) {
    if (u >= m_numVertices || v >= m_numVertices) {
        throw std::runtime_error("addEdge: vertex out of range");
    }

    // Проверка на дубликат (опционально, для простоты пропустим)
    m_adjacency[u].push_back(v);
    m_weights[u].push_back(w);

    m_adjacency[v].push_back(u);
    m_weights[v].push_back(w);

    m_numEdges++;
}

bool Graph::hasEdge(VertexId u, VertexId v) const {
    if (u >= m_numVertices || v >= m_numVertices) return false;

    const auto& neighbors = m_adjacency[u];
    return std::find(neighbors.begin(), neighbors.end(), v) != neighbors.end();
}

Weight Graph::getEdgeWeight(VertexId u, VertexId v) const {
    if (u >= m_numVertices || v >= m_numVertices) return 0.0f;

    const auto& neighbors = m_adjacency[u];
    const auto& weights = m_weights[u];

    for (size_t i = 0; i < neighbors.size(); ++i) {
        if (neighbors[i] == v) {
            return weights[i];
        }
    }
    return 0.0f;
}

bool Graph::isValid() const {
    if (m_adjacency.size() != static_cast<size_t>(m_numVertices)) return false;
    if (m_weights.size() != static_cast<size_t>(m_numVertices)) return false;

    for (VertexId v = 0; v < m_numVertices; ++v) {
        if (m_adjacency[v].size() != m_weights[v].size()) return false;
    }
    return true;
}

void Graph::printStats() const {
    std::cout << "Graph stats:\n";
    std::cout << "  Vertices: " << m_numVertices << "\n";
    std::cout << "  Edges: " << m_numEdges << "\n";

    size_t maxDeg = 0;
    size_t minDeg = m_numVertices;
    double avgDeg = 0.0;

    for (VertexId v = 0; v < m_numVertices; ++v) {
        size_t deg = m_adjacency[v].size();
        avgDeg += deg;
        if (deg > maxDeg) maxDeg = deg;
        if (deg < minDeg) minDeg = deg;
    }
    avgDeg /= m_numVertices;

    std::cout << "  Degree: min=" << minDeg << ", max=" << maxDeg
              << ", avg=" << avgDeg << "\n";
}

} // namespace clustering