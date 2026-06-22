#pragma once
#include "IClusteringAlgorithm.hpp"
#include <vector>
#include <map>

namespace clustering {

    class LouvainClustering : public IClusteringAlgorithm {
    public:
        LouvainClustering();

        ClusteringResult computeClusters(
            const Graph& graph,
            const ClusteringParams& params
        ) override;

        std::string name() const override { return "Louvain"; }
        std::string description() const override {
            return "Louvain modularity maximization with hierarchical clustering";
        }

        void setMaxPasses(int passes) { m_maxPasses = passes; }
        void setResolution(double resolution) { m_resolution = resolution; }

    private:
        struct AggregatedGraph {
            int numVertices;
            std::vector<std::map<int, double>> adjacency;
            std::vector<double> selfLoops;
            std::vector<double> totalEdgeWeight;
        };

        double computeModularity(
            const Graph& graph,
            const std::vector<int>& community,
            double totalEdgeWeight
        );

        ClusteringResult postProcess(
            const Graph& graph,
            const std::vector<int>& community,
            int maxClusterSize
        );

        int m_maxPasses = 10;
        double m_resolution = 1.0;

        // Хранилище для агрегированных графов
        std::vector<Graph> m_aggregatedGraphs;
    };

} // namespace clustering