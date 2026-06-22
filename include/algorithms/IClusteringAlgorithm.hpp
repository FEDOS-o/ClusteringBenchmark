#pragma once
#include "../core/Graph.hpp"
#include "../core/Cluster.hpp"
#include <string>
#include <memory>

namespace clustering {

    struct ClusteringParams {
        int maxClusterSize = 64;
        int minClusterSize = 1;
        bool enforceWarpMultiple = true;
        int localOptimizationIters = 3;
        bool useIncremental = false;
        float qualityTolerance = 0.01f;

        std::string toString() const {
            return "maxSize=" + std::to_string(maxClusterSize) +
                   ", localIters=" + std::to_string(localOptimizationIters) +
                   ", warpAlign=" + std::to_string(enforceWarpMultiple);
        }
    };

    class IClusteringAlgorithm {
    public:
        virtual ~IClusteringAlgorithm() = default;

        virtual ClusteringResult computeClusters(
            const Graph& graph,
            const ClusteringParams& params
        ) = 0;

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