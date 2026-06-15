#pragma once
#include "IClusteringAlgorithm.hpp"

namespace clustering {

    class LouvainClustering : public IClusteringAlgorithm {
    public:
        ClusteringResult computeClusters(
            const Graph& graph,
            const ClusteringParams& params
        ) override;

        std::string name() const override { return "Louvain"; }
        std::string description() const override {
            return "Louvain modularity maximization";
        }

        void setMaxPasses(int passes) { m_maxPasses = passes; }

    private:
        int m_maxPasses = 10;
    };

} // namespace clustering