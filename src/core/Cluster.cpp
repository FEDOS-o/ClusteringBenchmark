#include "core/Cluster.hpp"
#include <sstream>

namespace clustering {

    std::string Cluster::toString() const {
        std::stringstream ss;
        ss << "Cluster[" << id << "]: size=" << vertices.size();
        if (vertices.size() <= 10) {
            ss << " vertices=[";
            for (size_t i = 0; i < vertices.size(); ++i) {
                if (i > 0) ss << ",";
                ss << vertices[i];
            }
            ss << "]";
        }
        return ss.str();
    }

} // namespace clustering