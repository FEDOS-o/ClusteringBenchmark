#pragma once
#include <cstdint>
#include <vector>
#include <utility>

namespace clustering {

    using VertexId = int32_t;
    using EdgeId = int64_t;
    using Weight = float;

    struct Edge {
        VertexId u;
        VertexId v;
        Weight weight = 1.0f;
    };

    using EdgeList = std::vector<Edge>;

} // namespace clustering