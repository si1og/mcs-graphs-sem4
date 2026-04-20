#include "lab1.h"
#include <algorithm>
#include <queue>
#include <limits>
#include <cmath>

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
    double u;
    do {
        u = m_uniformDist(m_rng);
    } while (u >= 1);

    return m_weibullY0 + m_weibullA * std::pow(-std::log(1 - u), 1 / m_weibullC);
}

std::vector<int> GeneratorGraph::m_generateDegreeSequence() {
    std::vector<int> degrees(m_vertexCount);
    for (int i = 0; i < m_vertexCount; ++i) {
        degrees[i] = std::max(1, static_cast<int>(std::round(m_sampleWeibull())));
        degrees[i] = std::min(degrees[i], m_vertexCount - 1);
    }
    return degrees;
}

void GeneratorGraph::m_ensureConnected() {
    Matrix A(m_vertexCount, m_vertexCount);
    for (int i = 0; i < m_vertexCount; ++i) {
        for (int j = 0; j < m_vertexCount; ++j) {
            if (m_adjacencyMatrix(i, j) != 0 || m_adjacencyMatrix(j, i) != 0) {
                A(i, j) = 1;
            }
        }
    }

    std::vector<bool> reachable(m_vertexCount, false);
    reachable[0] = true;

    Matrix Ak = A;
    for (int k = 1; k < m_vertexCount; ++k) {
        for (int j = 0; j < m_vertexCount; ++j) {
            if (Ak(0, j) > 0) reachable[j] = true;
        }
        if (k < m_vertexCount - 1) Ak = Ak * A;
    }

    for (int i = 1; i < m_vertexCount; ++i) {
        if (!reachable[i]) {
            m_adjacencyMatrix(i - 1, i) = 1;
            reachable[i] = true;
        }
    }
}

void GeneratorGraph::generate() {
    m_adjacencyMatrix = Matrix(m_vertexCount, m_vertexCount);

    auto degrees = m_generateDegreeSequence();

    // рёбра только от i к j, где i < j
    //
    // любое ребро идёт от меньшего индекса к большему. Значит, любой путь v_0 -> v_1 -> v_2
    // -> ... обязан иметь строго возрастающую последовательность индексов, поэтому цикла быть не может
    for (int i = 0; i < m_vertexCount; ++i) {
        std::vector<int> candidates;
        for (int j = i + 1; j < m_vertexCount; ++j) {
            candidates.push_back(j);
        }
        std::shuffle(candidates.begin(), candidates.end(), m_rng);

        int edgesAdded = 0;
        for (int j : candidates) {
            if (edgesAdded >= degrees[i]) break;
            m_adjacencyMatrix(i, j) = 1;
            ++edgesAdded;
        }
    }

    m_ensureConnected();
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
                double w = std::round(m_sampleWeibull());
                if (w < 1) w = 1;

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
}

Matrix GeneratorGraph::shimbell(int steps, bool findMin) const {
    const double NO_EDGE = findMin
        ? std::numeric_limits<double>::infinity()
        : -std::numeric_limits<double>::infinity();

    Matrix W(m_vertexCount, m_vertexCount, NO_EDGE);

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
    std::vector<bool> visited(m_vertexCount, false);
    std::queue<int> q;
    q.push(from);
    visited[from] = true;

    while (!q.empty()) {
        int v = q.front(); q.pop();
        if (v == to) return true;
        for (int u = 0; u < m_vertexCount; ++u) {
            if (!visited[u] && m_adjacencyMatrix(v, u) != 0) {
                visited[u] = true;
                q.push(u);
            }
        }
    }

    return false;
}

int GeneratorGraph::countRoutes(int from, int to) const {
    std::vector<int> dp(m_vertexCount, 0);
    dp[from] = 1;

    for (int v = from; v < m_vertexCount; ++v) {
        if (dp[v] == 0) continue;
        for (int u = v + 1; u < m_vertexCount; ++u) {
            if (m_adjacencyMatrix(v, u) != 0) {
                dp[u] += dp[v];
            }
        }
    }

    return dp[to];
}
