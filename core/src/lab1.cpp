#include "lab1.h"
#include <algorithm>
// #include <queue>
#include <iostream>
#include <limits>
#include <cmath>
#include <vector>

GeneratorGraph::GeneratorGraph(int vertexCount,
                               double weibullA,
                               double weibullC,
                               double weibullY0)
    : Graph(vertexCount),
      m_rng(std::random_device{}()),
      m_uniformDist(0, 1),
      m_weibullA(weibullA),
      m_weibullC(weibullC),
      m_weibullY0(weibullY0),
      m_diameter(0)
{}

double GeneratorGraph::m_sampleWeibull() {
double r;
do {
    r = m_uniformDist(m_rng);
} while (r >= 1);

return m_weibullY0 + m_weibullA * std::pow(-std::log(1 - r), 1 / m_weibullC);
}

std::vector<int> GeneratorGraph::m_generateDegreeSequence() {
    std::vector<int> degrees(m_vertexCount);

    bool valid;
    do {
        valid = true;
        for (int i = 0; i < m_vertexCount; ++i) {
            double raw = m_sampleWeibull();
            degrees[i] = static_cast<int>(std::round(raw * (m_vertexCount - 1)));

            if (degrees[i] > m_vertexCount - 1 || degrees[i] == 0) {
                valid = false;
                break;
            }
        }
    } while (!valid);

    return degrees;
}

void GeneratorGraph::testDistribution() {
    std::vector<int> degrees(m_vertexCount);

    for (int i = 0; i < 30; ++i) {
        std::cout << "Генерация №" << (i + 1) << ":\n";
        for (int j = 0; j < m_vertexCount; ++j) {
            degrees = m_generateDegreeSequence();
        }

        for (auto e : degrees) std::cout << e << " ";
        std::cout << "\n";
    }
}

void GeneratorGraph::generate() {
    m_adjacencyMatrix = Matrix(m_vertexCount, m_vertexCount);
    auto degrees = m_generateDegreeSequence();

    // гарантирует связность
    for (int i = 0; i < m_vertexCount - 1; ++i) {
        m_adjacencyMatrix(i, i + 1) = 1;
    }

    // добор оставшихся рёбер
    for (int i = 0; i < m_vertexCount; ++i) {
        int alreadyAdded = (i < m_vertexCount - 1) ? 1 : 0;

        std::vector<int> candidates;
        for (int j = i + 1; j < m_vertexCount; ++j) {
            // если ещё не добавлено
            if (m_adjacencyMatrix(i, j) == 0) {
                candidates.push_back(j);
            }
        }
        std::shuffle(candidates.begin(), candidates.end(), m_rng);

        int needToAdd = std::max(0, degrees[i] - alreadyAdded);
        int edgesAdded = 0;
        for (int j : candidates) {
            if (edgesAdded >= needToAdd) break;
            m_adjacencyMatrix(i, j) = 1;
            ++edgesAdded;
        }
    }
}

void GeneratorGraph::computeEccentricities() {
    const Matrix& A = m_adjacencyMatrix;

    const int UNREACHABLE = -1;
    Matrix dist(m_vertexCount, m_vertexCount, UNREACHABLE);
    for (int i = 0; i < m_vertexCount; ++i) dist(i, i) = 0;

    Matrix Ak = A;
    for (int k = 1; k < m_vertexCount; ++k) {
        for (int i = 0; i < m_vertexCount; ++i) {
            for (int j = 0; j < m_vertexCount; ++j) {
                if (i != j && dist(i, j) == UNREACHABLE && Ak(i, j) > 0) {
                    dist(i, j) = k;
                }
            }
        }
        if (k < m_vertexCount - 1) Ak = Ak * A;
    }

    m_eccentricities.assign(m_vertexCount, 0);
    for (int i = 0; i < m_vertexCount; ++i) {
        int maxDist = 0;
        for (int j = 0; j < m_vertexCount; ++j) {
            if (i != j && dist(i, j) != UNREACHABLE) {
                maxDist = std::max(maxDist, static_cast<int>(dist(i, j)));
            }
        }
        m_eccentricities[i] = maxDist;
    }

    m_diameter = *std::max_element(m_eccentricities.begin(), m_eccentricities.end());
    int radius = *std::min_element(m_eccentricities.begin(), m_eccentricities.end());

    m_centerVertices.clear();
    m_diametralVertices.clear();
    for (int i = 0; i < m_vertexCount; ++i) {
        if (m_eccentricities[i] == radius)    m_centerVertices.push_back(i);
        if (m_eccentricities[i] == m_diameter) m_diametralVertices.push_back(i);
    }
}

std::vector<double> GeneratorGraph::getEccentricities() const { return m_eccentricities; }
std::vector<int> GeneratorGraph::getCenterVertices() const { return m_centerVertices; }
std::vector<int> GeneratorGraph::getDiametralVertices() const { return m_diametralVertices; }
int GeneratorGraph::getDiameter() const { return m_diameter; }

void GeneratorGraph::generateWeightMatrix(WeightMode mode) {
    m_weightMatrix = Matrix(m_vertexCount, m_vertexCount);

    std::uniform_int_distribution<int> signDist(0, 1);

    for (int i = 0; i < m_vertexCount; ++i) {
        for (int j = 0; j < m_vertexCount; ++j) {
            if (m_adjacencyMatrix(i, j) != 0) {
                const int WEIGHT_MATRIX_DIST_VALUE = 20;
                double w = std::round(m_sampleWeibull() * WEIGHT_MATRIX_DIST_VALUE);

                switch (mode) {
                    case WeightMode::Positive:
                        m_weightMatrix(i, j) = w;
                        break;
                    case WeightMode::Negative:
                        m_weightMatrix(i, j) = -w;
                        break;
                    case WeightMode::Mixed:
                        m_weightMatrix(i, j) = signDist(m_rng) ? w : -w;
                        break;
                }
            }
        }
    }

    isMatrixInit.weight = true;
}

//TODO: пофиксить матрицу шиблелла при steps = 0
// посвторить определения из учебника
Matrix GeneratorGraph::shimbell(int steps, bool findMin) const {
    const double NO_EDGE = findMin
        ? std::numeric_limits<double>::infinity()
        : -std::numeric_limits<double>::infinity();

    Matrix W(m_vertexCount, m_vertexCount, NO_EDGE);

    if (steps == 0) {
        return W;
    }

    for (int i = 0; i < m_vertexCount; ++i) {
        for (int j = 0; j < m_vertexCount; ++j) {
            if (m_adjacencyMatrix(i, j) != 0) {
                W(i, j) = m_weightMatrix(i, j);
            }
        }
    }

    Matrix result = W;
    for (int s = 1; s < steps; ++s) {
        result = findMin ? result.shimbellMin(W) : result.shimbellMax(W);
    }

    return result;
}

bool GeneratorGraph::hasRoute(int from, int to) const {
    if (from == to) return true;

    Matrix Ak = m_adjacencyMatrix;
    for (int k = 1; k < m_vertexCount; ++k) {
        if (Ak(from, to) > 0) return true;
        if (k < m_vertexCount - 1) Ak = Ak * m_adjacencyMatrix;
    }
    return false;
}

int GeneratorGraph::countRoutes(int from, int to) const {
    if (from == to) return 1;

    int total = 0;
    Matrix Ak = m_adjacencyMatrix;
    for (int k = 1; k < m_vertexCount; ++k) {
        total += static_cast<int>(Ak(from, to));
        if (k < m_vertexCount - 1) Ak = Ak * m_adjacencyMatrix;
    }
    return total;
}
