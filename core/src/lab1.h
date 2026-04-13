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
                   double weibullA = constants::WEIBULL_SCALE,
                   double weibullC = constants::WEIBULL_SHAPE);

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

private:
    std::mt19937 m_rng;
    std::weibull_distribution<double> m_weibullDist;

    std::vector<double> m_eccentricities;
    std::vector<int> m_centerVertices;
    std::vector<int> m_diametralVertices;
    int m_diameter = 0;

    std::vector<int> m_generateDegreeSequence();
    void m_ensureConnected();
};
