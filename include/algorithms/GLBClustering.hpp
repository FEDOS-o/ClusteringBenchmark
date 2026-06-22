#pragma once
#include "IClusteringAlgorithm.hpp"
#include <vector>

namespace clustering {

    class GLBClustering : public IClusteringAlgorithm {
    public:
        ClusteringResult computeClusters(
            const Graph& graph,
            const ClusteringParams& params
        ) override;

        bool supportsIncremental() const override { return true; }

        ClusteringResult updateClusters(
            const Graph& graph,
            const ClusteringResult& previous,
            const ClusteringParams& params
        ) override;

        std::string name() const override { return "GLB"; }
        std::string description() const override {
            return "Greedy-Local-Balance: hybrid clustering with balance optimization";
        }

    private:
        struct MoveGain {
            VertexId vertex;
            int fromCluster;
            int toCluster;
            float cutDelta;
            float balanceImprovement;
            float totalScore;
        };

        ClusteringResult greedyInitialization(const Graph& graph, int maxSize);

        void localOptimization(
            const Graph& graph,
            ClusteringResult& clusters,
            int maxIterations,
            float tolerance
        );

        MoveGain evaluateMove(
            const Graph& graph,
            const ClusteringResult& clusters,
            const std::vector<int>& vertexToCluster,
            VertexId vertex,
            int currentClusterId
        );

        void applyMove(
            ClusteringResult& clusters,
            std::vector<int>& vertexToCluster,
            const MoveGain& move
        );

        void enforceWarpMultiple(ClusteringResult& clusters);
    };

} // namespace clustering