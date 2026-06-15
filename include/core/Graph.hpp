#pragma once
#include "Types.hpp"
#include <vector>
#include <unordered_map>
#include <string>

namespace clustering {

    class Graph {
    public:
        // Конструкторы
        Graph() = default;
        explicit Graph(VertexId numVertices);
        Graph(VertexId numVertices, const EdgeList& edges);

        // Построение из рёбер
        void buildFromEdges(VertexId numVertices, const EdgeList& edges);

        // Добавление вершины/ребра
        void addVertex();
        void addEdge(VertexId u, VertexId v, Weight w = 1.0f);

        // Доступ к данным
        VertexId getNumVertices() const { return m_numVertices; }
        EdgeId getNumEdges() const { return m_numEdges; }

        // Список смежности: для вершины v возвращаем vector соседей
        const std::vector<VertexId>& getNeighbors(VertexId v) const {
            return m_adjacency[v];
        }

        // С весами
        const std::vector<Weight>& getNeighborWeights(VertexId v) const {
            return m_weights[v];
        }

        // Степень вершины
        size_t getDegree(VertexId v) const { return m_adjacency[v].size(); }

        // Проверка наличия ребра (O(degree), норм для небольших графов)
        bool hasEdge(VertexId u, VertexId v) const;

        // Получение веса ребра
        Weight getEdgeWeight(VertexId u, VertexId v) const;

        // Валидация
        bool isValid() const;

        // Для отладки
        void printStats() const;

    private:
        VertexId m_numVertices = 0;
        EdgeId m_numEdges = 0;

        // Список смежности: для каждой вершины — вектор соседей
        std::vector<std::vector<VertexId>> m_adjacency;

        // Веса рёбер (параллельный массив к m_adjacency)
        std::vector<std::vector<Weight>> m_weights;

        // Опционально: быстрый поиск ребра (можно и без него)
        // std::vector<std::unordered_map<VertexId, Weight>> m_edgeMap;
    };

} // namespace clustering