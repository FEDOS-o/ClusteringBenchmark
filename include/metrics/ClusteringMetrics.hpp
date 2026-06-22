#pragma once
#include "../core/Types.hpp"
#include "../core/Cluster.hpp"
#include <string>
#include <vector>
#include <map>

namespace clustering {

    struct ClusteringMetrics {
        // === Базовые метрики ===
        int numClusters = 0;          // C — количество кластеров
        int numVertices = 0;          // N — количество ограничений (вершин)
        int numEdges = 0;             // |E| — количество рёбер в графе ограничений

        // === Размеры кластеров ===
        double minClusterSize = 0;
        double maxClusterSize = 0;
        double meanClusterSize = 0;   // S_avg = N / C
        double stdDevClusterSize = 0;
        double coefficientOfVariation = 0;  // CV = σ/μ — баланс
        double imbalance = 0;               // max/μ - 1 — дисбаланс

        // === Метрики качества разбиения ===
        double cut = 0;                     // Разрез (межкластерные рёбра)
        double modularity = 0;              // Q — модулярность (качество сообществ)
        double internalDensity = 0;         // Плотность внутри кластеров

        // === Метрики для параллельного выполнения ===
        int minColorsEstimate = 0;          // Δ(G_C) + 1 — нижняя оценка
        int greedyColorsEstimate = 0;       // K — реальная оценка цветов (жадная раскраска)
        double parallelismPotential = 0;    // C / K — сколько кластеров в среднем на цвет

        // === GPU-специфичные метрики (оценка) ===
        double warpEfficiency = 0;          // Насколько размеры кластеров кратны 32
        double loadBalanceScore = 0;        // 1 / (1 + CV) — оценка балансировки

        // === Дополнительные метрики ===
        int maxDegreeInClusterGraph = 0;    // Δ(G_C) — максимальная степень в графе кластеров
        double avgDegreeInClusterGraph = 0; // Средняя степень в графе кластеров

        // Гистограммы для анализа
        std::map<int, int> clusterSizeHistogram;

        // === Сериализация ===
        std::string toCSVHeader() const;
        std::string toCSVRow() const;
        void print() const;
    };

} // namespace clustering