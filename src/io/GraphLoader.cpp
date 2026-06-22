#include "io/GraphLoader.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace clustering {

Graph GraphLoader::loadFromCSV(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }

    EdgeList edges;
    VertexId maxVertex = 0;
    std::string line;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::stringstream ss(line);
        VertexId u, v;
        char comma;
        ss >> u >> comma >> v;

        maxVertex = std::max(maxVertex, u);
        maxVertex = std::max(maxVertex, v);
        edges.push_back({u, v, 1.0f});
    }

    Graph graph;
    graph.buildFromEdges(maxVertex + 1, edges);
    return graph;
}

Graph GraphLoader::loadFromSimple(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }

    VertexId numVertices;
    EdgeId numEdges;
    file >> numVertices >> numEdges;

    EdgeList edges;
    for (EdgeId i = 0; i < numEdges; ++i) {
        VertexId u, v;
        Weight w = 1.0f;
        file >> u >> v;
        // Если есть третий параметр — вес
        if (file.peek() != '\n' && file.peek() != '\r') {
            file >> w;
        }
        edges.push_back({u, v, w});
    }

    Graph graph;
    graph.buildFromEdges(numVertices, edges);
    return graph;
}

void GraphLoader::saveToSimple(const Graph& graph, const std::string& filename) {
    std::ofstream file(filename);

    file << graph.getNumVertices() << " " << graph.getNumEdges() << "\n";

    for (VertexId u = 0; u < graph.getNumVertices(); ++u) {
        const auto& neighbors = graph.getNeighbors(u);
        for (VertexId v : neighbors) {
            if (u < v) {
                file << u << " " << v << " 1.0\n";
            }
        }
    }
}

void GraphLoader::saveToCSV(const Graph& graph, const std::string& filename) {
    std::ofstream file(filename);

    for (VertexId u = 0; u < graph.getNumVertices(); ++u) {
        const auto& neighbors = graph.getNeighbors(u);
        for (VertexId v : neighbors) {
            if (u < v) {
                file << u << "," << v << "\n";
            }
        }
    }
}

void GraphLoader::validateGraph(const Graph& graph) {
    if (!graph.isValid()) {
        throw std::runtime_error("Invalid graph structure");
    }
}

} // namespace clustering