#pragma once
#include "IClusteringAlgorithm.hpp"

namespace clustering {

    class GreedyClustering : public IClusteringAlgorithm {
    public:
        ClusteringResult computeClusters(
            const Graph& graph,
            const ClusteringParams& params
        ) override;

        std::string name() const override { return "GreedyBFS"; }
        std::string description() const override {
            return "Greedy BFS clustering with size limit";
        }
    };

} // namespace clustering