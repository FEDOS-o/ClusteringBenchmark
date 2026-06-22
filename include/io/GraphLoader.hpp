#pragma once
#include "../core/Graph.hpp"
#include <string>

namespace clustering {

    class GraphLoader {
    public:
        // Загрузка из CSV (список рёбер: u,v)
        static Graph loadFromCSV(const std::string& filename);

        // Загрузка из простого текстового формата:
        // первая строка: numVertices numEdges
        // следующие строки: u v [weight]
        static Graph loadFromSimple(const std::string& filename);

        // Сохранение в простой текстовый формат
        static void saveToSimple(const Graph& graph, const std::string& filename);

        // Сохранение в CSV
        static void saveToCSV(const Graph& graph, const std::string& filename);

    private:
        static void validateGraph(const Graph& graph);
    };

} // namespace clustering