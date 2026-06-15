#pragma once
#include "Types.hpp"
#include <vector>
#include <string>

namespace clustering {

    struct Cluster {
        int id;
        std::vector<VertexId> vertices;

        size_t size() const { return vertices.size(); }
        std::string toString() const;
    };

    using ClusteringResult = std::vector<Cluster>;

} // namespace clustering