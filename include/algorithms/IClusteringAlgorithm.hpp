#pragma once
#include "../core/Graph.hpp"
#include "../core/Cluster.hpp"
#include <string>
#include <memory>

namespace clustering {

    struct ClusteringParams {
        int maxClusterSize = 64;           // S_max
        int minClusterSize = 1;
        bool enforceWarpMultiple = true;   // кратность 32
        int localOptimizationIters = 3;    // для GLB
        bool useIncremental = false;
        float qualityTolerance = 0.01f;

        std::string toString() const;
    };

    class IClusteringAlgorithm {
    public:
        virtual ~IClusteringAlgorithm() = default;

        // Основной метод
        virtual ClusteringResult computeClusters(
            const Graph& graph,
            const ClusteringParams& params
        ) = 0;

        // Инкрементальное обновление (опционально)
        virtual ClusteringResult updateClusters(
            const Graph& graph,
            const ClusteringResult& previous,
            const ClusteringParams& params
        ) {
            return computeClusters(graph, params);
        }

        virtual bool supportsIncremental() const { return false; }
        virtual std::string name() const = 0;
        virtual std::string description() const = 0;

        double getLastExecutionTimeMs() const { return m_lastTimeMs; }

    protected:
        void setLastExecutionTime(double ms) { m_lastTimeMs = ms; }

    private:
        double m_lastTimeMs = 0.0;
    };

    using AlgorithmPtr = std::unique_ptr<IClusteringAlgorithm>;

} // namespace clustering