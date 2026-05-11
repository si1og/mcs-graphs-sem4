#pragma once

#include "graph.h"
#include "constants.h"
#include "flows.h"
#include <random>
#include <vector>
#include <queue>
#include <limits>
#include <algorithm>
#include <functional>

enum class WeightMode {
    Positive,
    Negative,
    Mixed
};

struct ShortestPathResult {
    std::vector<double> T;
    std::vector<int> H;
    std::vector<int> path;

    double distance = 0;
    int iterations = 0;

    bool hasPath = false;
    bool hasNegativeCycle = false;
};

struct ArticulationPointsResult {
    std::vector<int> points;
    int iterations = 0;
};

class GeneratorGraph : public Graph {
public:
    GeneratorGraph(int vertexCount);

    void generate();

    void computeEccentricities();
    std::vector<double> getEccentricities() const;
    std::vector<int> getCenterVertices() const;
    std::vector<int> getDiametralVertices() const;
    int getDiameter() const;

    void generateWeightMatrix(WeightMode mode);
    Matrix shimbell(int steps, bool findMin) const;

    bool hasRoute(int from, int to) const;
    int countRoutes(int from, int to) const;
    void testDistribution();

    //lab2
    ArticulationPointsResult findArticulationPoints() const;
    ShortestPathResult dijkstraNegative(int s, int t) const;
    ShortestPathResult dijkstraNegative(int s,
                                        int t,
                                        const Matrix& adjacency,
                                        const Matrix& weight) const;

    // lab3
    void generateCapacityAndCostMatrices();

    const Matrix& getCapacityMatrix() const;
    const Matrix& getCostMatrix() const;

    void printCapacityMatrix() const;
    void printCostMatrix() const;

    bool isCapacityMatrixGenerated() const;
    bool isCostMatrixGenerated() const;

    // Потоковые алгоритмы
    MaxFlowResult fordFulkerson(int source, int sink) const;

    MinCostFlowResult minCostFlow(
        int source,
        int sink,
        int requiredFlow
    ) const;

private:
    std::mt19937 m_rng;
    std::uniform_real_distribution<double> m_uniformDist;

    std::vector<double> m_eccentricities;
    std::vector<int> m_centerVertices;
    std::vector<int> m_diametralVertices;
    int m_diameter = 0;

    double m_sampleWeibull(const WeibullParams& parameters);
    std::vector<int> m_generateDegreeSequence();

    //lab2
    void m_dfsArticulation(int v,
                           int parent,
                           std::vector<int>& tin,
                           std::vector<int>& low,
                           std::vector<bool>& visited,
                           std::vector<bool>& isAP,
                           int& timer,
                           int& iterations) const;
    std::vector<int> m_restorePath(int s, int t, const std::vector<int>& H) const;

    // lab3
    Matrix m_capacityMatrix;
    Matrix m_costMatrix;
};
