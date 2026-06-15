#include "core/Graph.hpp"

#include <algorithm>
#include <unordered_set>
#include <stdexcept>

namespace clustering {

    Graph::Graph(VertexId numVertices)
        : m_numVertices(numVertices)
        , m_adjacency(numVertices)
    {}

    Graph::Graph(VertexId numVertices, const EdgeList& edges) {
        buildFromEdges(numVertices, edges);
    }

    void Graph::buildFromEdges(VertexId numVertices, const EdgeList& edges) {
        m_numVertices = numVertices;
        m_numEdges = edges.size();
        m_adjacency.assign(numVertices, {});

        for (const auto& e : edges) {
            if (e.u >= numVertices || e.v >= numVertices) {
                throw std::runtime_error("Edge vertex out of range");
            }
            m_adjacency[e.u].push_back(e.v);
            m_adjacency[e.v].push_back(e.u);  // неориентированный граф
        }

        // Можно отсортировать для детерминизма (опционально)
        for (auto& neighbors : m_adjacency) {
            std::sort(neighbors.begin(), neighbors.end());
            neighbors.erase(std::unique(neighbors.begin(), neighbors.end()), neighbors.end());
        }
    }

    bool Graph::hasEdge(VertexId u, VertexId v) const {
        if (u >= m_numVertices || v >= m_numVertices) return false;
        const auto& neighbors = m_adjacency[u];
        return std::find(neighbors.begin(), neighbors.end(), v) != neighbors.end();
    }

    bool Graph::isValid() const {
        if (m_numVertices == 0) return false;
        if (m_adjacency.size() != static_cast<size_t>(m_numVertices)) return false;
        return true;
    }

} // namespace clustering