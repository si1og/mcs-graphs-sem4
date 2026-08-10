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
#include <set>

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

// lab4
struct SpanningTreesResult {
    Matrix kirchhoffMatrix;
    Matrix cofactorMatrix;

    double cofactorDeterminant = 0;
    long long count = 0;

    SpanningTreesResult(int vertexCount)
        : kirchhoffMatrix(vertexCount, vertexCount, 0)
        , cofactorMatrix(
              std::max(0, vertexCount - 1),
              std::max(0, vertexCount - 1),
              0
          )
    {}
};

struct Edge {
    int from = 0;
    int to = 0;

    Edge() = default;
    Edge(int from, int to) : from(from), to(to) {};

    bool operator<(const Edge& other) const {
        return from < other.from ||
               (from == other.from && to < other.to);
    }

    bool operator==(const Edge& other) const {
        return from == other.from && to == other.to;
    }
};

struct WeightedEdge {
    int from = 0;
    int to = 0;
    double weight = 0;
};

struct PruferCodeItem {
    int vertex = 0;
    double weight = 0;
};

struct KruskalResult {
    std::vector<WeightedEdge> edges;
    Matrix treeMatrix;
    double totalWeight = 0;
    bool success = false;

    KruskalResult(int vertexCount)
        : treeMatrix(
            vertexCount,
            vertexCount,
            std::numeric_limits<double>::infinity()
        )
    {}
};

struct KruskalMinimumSpanningTreeResult {
    KruskalResult kruskal;

    std::vector<PruferCodeItem> pruferCode;
    Matrix decodedTreeMatrix;
    bool pruferRoundTripSuccess = false;

    KruskalMinimumSpanningTreeResult(KruskalResult&& other, int vertexCount)
        : kruskal(std::move(other)),
          decodedTreeMatrix(
              vertexCount,
              vertexCount,
              std::numeric_limits<double>::infinity()
          )
    {}
};

struct EdmondsBlossomResult {
    std::vector<WeightedEdge> edges;
    Matrix edgeMatrix;

    bool success = true;
    bool hasWeights = false;

    EdmondsBlossomResult(int vertexCount)
        : edgeMatrix(vertexCount, vertexCount, 0)
    {}
};

// lab5
struct CheckIfEulerianGraphResult {
    bool isEulerian = false;
    bool transformationCompleted = true;
    bool resultIsConnected = true;
    std::vector<Edge> addedEdges;
    std::vector<Edge> removedEdges;
    std::vector<int> eulerianCycle;
    Matrix eulerianAdjacencyMatrix;

    CheckIfEulerianGraphResult(
        bool isEulerian,
        bool transformationCompleted,
        bool resultIsConnected,
        std::vector<Edge>&& added,
        std::vector<Edge>&& removed,
        std::vector<int>&& cycle,
        Matrix&& adjacency
    )
        : isEulerian(isEulerian),
          transformationCompleted(transformationCompleted),
          resultIsConnected(resultIsConnected),
          addedEdges(std::move(added)),
          removedEdges(std::move(removed)),
          eulerianCycle(std::move(cycle)),
          eulerianAdjacencyMatrix(std::move(adjacency))
    {}
};

using FleuryResult = std::vector<int>;
using GraphCut = std::set<Edge>;

struct FundamentalCut {
    Edge removedTreeEdge;
    GraphCut cut;
};

struct FundamentalCutsResult {
    KruskalResult spanningTree;
    std::vector<FundamentalCut> fundamentalCuts;
    bool success = false;

    explicit FundamentalCutsResult(int vertexCount)
        : spanningTree(vertexCount)
    {}
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
    ShortestPathResult dijkstraNegative(int s, int t, const Matrix& adjacency) const;
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

    // lab4
    SpanningTreesResult countSpanningTreesKirchhoff() const;
    KruskalMinimumSpanningTreeResult kruskalMinimumSpanningTree() const;
    EdmondsBlossomResult edmondsBlossomInOriginalGraph() const;
    EdmondsBlossomResult edmondsBlossomInMinimumSpanningTree() const;

    //lab5
    CheckIfEulerianGraphResult checkIfEulerianGraph(
        bool allowDisconnectedResult = false
    ) const;

    FundamentalCutsResult buildFundamentalCutSystem() const;
    GraphCut symmetricDifferenceOfFundamentalCuts(
        const FundamentalCutsResult& system,
        const std::vector<int>& selectedCutIndices
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

    // lab4
    KruskalResult m_kruskalAlgorithm() const;
    EdmondsBlossomResult m_edmondsBlossom(
        const Matrix& adjacency,
        const Matrix& weights,
        bool hasWeights
    ) const;
    std::vector<PruferCodeItem> m_encodePruferCode(
        const std::vector<WeightedEdge>& treeEdges
    ) const;
    Matrix m_decodePruferCode(
        const std::vector<PruferCodeItem>& code
    ) const;

    // lab5
    FleuryResult m_flueryAlgorithm(const Matrix& adjacency) const;
    GraphCut m_symmetricDifference(
        const GraphCut& first,
        const GraphCut& second
    ) const;
};
