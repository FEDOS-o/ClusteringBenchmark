#pragma once
#include "../core/Graph.hpp"
#include <string>

namespace clustering {

    class GraphLoader {
    public:
        static Graph loadFromJSON(const std::string& filename);
        static Graph loadFromCSV(const std::string& filename);
        static void saveToJSON(const Graph& graph, const std::string& filename);

    private:
        static void validateGraph(const Graph& graph);
    };

} // namespace clustering