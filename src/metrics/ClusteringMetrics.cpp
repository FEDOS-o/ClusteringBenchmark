#include "metrics/ClusteringMetrics.hpp"
#include <sstream>
#include <iomanip>
#include <cmath>
#include <iostream>

namespace clustering {

std::string ClusteringMetrics::toCSVHeader() const {
    return "numClusters,numVertices,numEdges,"
           "minClusterSize,maxClusterSize,meanClusterSize,stdDevClusterSize,"
           "coefficientOfVariation,imbalance,"
           "cut,modularity,internalDensity,"
           "minColorsEstimate,greedyColorsEstimate,parallelismPotential,"
           "maxDegreeInClusterGraph,avgDegreeInClusterGraph,"
           "warpEfficiency,loadBalanceScore";
}

std::string ClusteringMetrics::toCSVRow() const {
    std::stringstream ss;
    ss << numClusters << ","
       << numVertices << ","
       << numEdges << ","
       << minClusterSize << ","
       << maxClusterSize << ","
       << meanClusterSize << ","
       << stdDevClusterSize << ","
       << coefficientOfVariation << ","
       << imbalance << ","
       << cut << ","
       << modularity << ","
       << internalDensity << ","
       << minColorsEstimate << ","
       << greedyColorsEstimate << ","
       << parallelismPotential << ","
       << maxDegreeInClusterGraph << ","
       << avgDegreeInClusterGraph << ","
       << warpEfficiency << ","
       << loadBalanceScore;
    return ss.str();
}

void ClusteringMetrics::print() const {
    std::cout << "=== Clustering Metrics ===\n";
    std::cout << "Clusters: " << numClusters << "\n";
    std::cout << "Vertices: " << numVertices << "\n";
    std::cout << "Edges: " << numEdges << "\n";
    std::cout << "\n--- Cluster Sizes ---\n";
    std::cout << "  min: " << minClusterSize << "\n";
    std::cout << "  max: " << maxClusterSize << "\n";
    std::cout << "  mean: " << meanClusterSize << "\n";
    std::cout << "  std: " << stdDevClusterSize << "\n";
    std::cout << "  CV: " << coefficientOfVariation << "\n";
    std::cout << "  imbalance: " << imbalance << "\n";
    std::cout << "\n--- Quality ---\n";
    std::cout << "  cut: " << cut << "\n";
    std::cout << "  modularity: " << modularity << "\n";
    std::cout << "  internal density: " << internalDensity << "\n";
    std::cout << "\n--- Parallelism ---\n";
    std::cout << "  min colors (Δ+1): " << minColorsEstimate << "\n";
    std::cout << "  greedy colors (K): " << greedyColorsEstimate << "\n";
    std::cout << "  parallelism potential (C/K): " << parallelismPotential << "\n";
    std::cout << "\n--- GPU Metrics ---\n";
    std::cout << "  warp efficiency: " << warpEfficiency << "\n";
    std::cout << "  load balance score: " << loadBalanceScore << "\n";
    std::cout << "\n--- Cluster Graph ---\n";
    std::cout << "  max degree: " << maxDegreeInClusterGraph << "\n";
    std::cout << "  avg degree: " << avgDegreeInClusterGraph << "\n";
}

} // namespace clustering