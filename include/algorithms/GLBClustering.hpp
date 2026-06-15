#pragma once
#include "IClusteringAlgorithm.hpp"

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
    };

} // namespace clustering