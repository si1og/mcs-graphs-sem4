#pragma once

#include "graph.h"
#include "constants.h"
#include <random>
#include <vector>

enum class WeightMode {
    Positive,
    Negative,
    Mixed
};

class GeneratorGraph : public Graph {
public:
    GeneratorGraph(int vertexCount,
                   double weibullA  = constants::WEIBULL_SCALE,
                   double weibullC  = constants::WEIBULL_SHAPE,
                   double weibullY0 = constants::WEIBULL_SHIFT);

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
    std::vector<int> findArticulationPoints() const;

private:
    std::mt19937 m_rng;
    std::uniform_real_distribution<double> m_uniformDist;

    double m_weibullA;
    double m_weibullC;
    double m_weibullY0;

    std::vector<double> m_eccentricities;
    std::vector<int> m_centerVertices;
    std::vector<int> m_diametralVertices;
    int m_diameter = 0;

    double m_sampleWeibull();
    std::vector<int> m_generateDegreeSequence();

    //lab2
    void m_dfsArticulation(int v,
                           int parent,
                           std::vector<int>& tin,
                           std::vector<int>& low,
                           std::vector<bool>& visited,
                           std::vector<bool>& isAP,
                           int& timer) const;
};
