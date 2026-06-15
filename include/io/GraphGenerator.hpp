#pragma once
#include "../core/Graph.hpp"
#include <random>

namespace clustering {

    class GraphGenerator {
    public:
        GraphGenerator(uint64_t seed = 42);

        static Graph generateGrid2D(int width, int height);
        static Graph generateGrid3D(int width, int height, int depth);
        Graph generateErdosRenyi(VertexId n, double p);
        Graph generateSmallWorld(VertexId n, int k, double beta);
        static Graph generateChain(VertexId n);
        static Graph generateComplete(VertexId n);

    private:
        std::mt19937 m_rng;
    };

} // namespace clustering