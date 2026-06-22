#include "io/ResultExporter.hpp"
#include <fstream>
#include <iomanip>
#include <sstream>
#include <chrono>

namespace clustering {

std::string ExperimentResult::toCSVRow() const {
    std::stringstream ss;
    ss << graphName << ","
       << algorithmName << ","
       << params.maxClusterSize << ","
       << params.localOptimizationIters << ","
       << params.enforceWarpMultiple << ","
       << executionTimeMs << ","
       << timestamp << ","
       << metrics.toCSVRow();
    return ss.str();
}

void ResultExporter::writeCSVHeader(std::ostream& os) {
    os << "graphName,algorithmName,maxClusterSize,localIters,enforceWarpMultiple,"
       << "executionTimeMs,timestamp,"
       << ClusteringMetrics().toCSVHeader() << "\n";
}

void ResultExporter::exportToCSV(const ExperimentResult& result, const std::string& filename) {
    std::ofstream file(filename);
    writeCSVHeader(file);
    file << result.toCSVRow() << "\n";
}

void ResultExporter::exportMultipleToCSV(
    const std::vector<ExperimentResult>& results,
    const std::string& filename
) {
    std::ofstream file(filename);
    writeCSVHeader(file);
    
    for (const auto& result : results) {
        file << result.toCSVRow() << "\n";
    }
}

void ResultExporter::exportToJSON(const ExperimentResult& result, const std::string& filename) {
    std::ofstream file(filename);
    file << "{\n";
    file << "  \"graphName\": \"" << result.graphName << "\",\n";
    file << "  \"algorithmName\": \"" << result.algorithmName << "\",\n";
    file << "  \"executionTimeMs\": " << result.executionTimeMs << ",\n";
    file << "  \"metrics\": {\n";
    file << "    \"numClusters\": " << result.metrics.numClusters << ",\n";
    file << "    \"cut\": " << result.metrics.cut << ",\n";
    file << "    \"modularity\": " << result.metrics.modularity << ",\n";
    file << "    \"coefficientOfVariation\": " << result.metrics.coefficientOfVariation << "\n";
    file << "  }\n";
    file << "}\n";
}

void ResultExporter::exportSummary(
    const std::vector<ExperimentResult>& results,
    const std::string& filename
) {
    std::ofstream file(filename);
    
    file << "=== Summary ===\n";
    file << "Total experiments: " << results.size() << "\n\n";
    
    file << "Algorithm\tAvg T_c(ms)\tAvg K\tAvg cut\tAvg CV\n";
    
    // Группировка по алгоритмам
    std::map<std::string, std::vector<const ExperimentResult*>> grouped;
    for (const auto& r : results) {
        grouped[r.algorithmName].push_back(&r);
    }
    
    for (const auto& [name, vec] : grouped) {
        double avgTime = 0.0;
        double avgK = 0.0;
        double avgCut = 0.0;
        double avgCV = 0.0;
        
        for (const auto* r : vec) {
            avgTime += r->executionTimeMs;
            avgK += r->metrics.greedyColorsEstimate;
            avgCut += r->metrics.cut;
            avgCV += r->metrics.coefficientOfVariation;
        }
        
        avgTime /= vec.size();
        avgK /= vec.size();
        avgCut /= vec.size();
        avgCV /= vec.size();
        
        file << name << "\t"
             << std::fixed << std::setprecision(2)
             << avgTime << "\t"
             << avgK << "\t"
             << avgCut << "\t"
             << avgCV << "\n";
    }
}

} // namespace clustering