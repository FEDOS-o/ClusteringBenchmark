#pragma once
#include "../metrics/ClusteringMetrics.hpp"
#include "../algorithms/IClusteringAlgorithm.hpp"
#include <string>
#include <vector>

namespace clustering {

    struct ExperimentResult {
        std::string graphName;
        std::string algorithmName;
        ClusteringParams params;
        ClusteringMetrics metrics;
        double executionTimeMs;
        std::string timestamp;

        std::string toCSVRow() const;
    };

    class ResultExporter {
    public:
        static void exportToCSV(const ExperimentResult& result, const std::string& filename);
        static void exportMultipleToCSV(const std::vector<ExperimentResult>& results, const std::string& filename);
        static void exportToJSON(const ExperimentResult& result, const std::string& filename);
        static void exportSummary(const std::vector<ExperimentResult>& results, const std::string& filename);
        static void writeCSVHeader(std::ostream& os);
    };

} // namespace clustering