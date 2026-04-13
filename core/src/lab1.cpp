#include "lab1.h"
#include <algorithm>
#include <queue>
#include <limits>
#include <cmath>

GeneratorGraph::GeneratorGraph(int vertexCount, double weibullA, double weibullC) : Graph(vertexCount),
    m_rng(std::random_device{}()),
    m_weibullDist(weibullC, weibullA),
    m_diameter(0)
{}

std::vector<int> GeneratorGraph::m_generateDegreeSequence() {
    std::vector<int> degrees(m_vertexCount);
    for (int i = 0; i < m_vertexCount; ++i) {
        degrees[i] = std::max(1, static_cast<int>(std::round(m_weibullDist(m_rng))));
        degrees[i] = std::min(degrees[i], m_vertexCount - 1);
    }
    return degrees;
}

void GeneratorGraph::m_ensureConnected() {
    // BFS по неориентированному варианту — проверяем слабую связность
    std::vector<bool> visited(m_vertexCount, false);
    std::queue<int> q;
    q.push(0);
    visited[0] = true;

    while (!q.empty()) {
        int v = q.front(); q.pop();
        for (int u = 0; u < m_vertexCount; ++u) {
            if (!visited[u] &&
                (m_adjacencyMatrix(v, u) != 0 || m_adjacencyMatrix(u, v) != 0)) {
                visited[u] = true;
                q.push(u);
            }
        }
    }

    // добавляем рёбра к недостижимым вершинам
    // i-1 → i сохраняет ацикличность (i-1 < i)
    for (int i = 1; i < m_vertexCount; ++i) {
        if (!visited[i]) {
            m_adjacencyMatrix(i - 1, i) = 1;
            visited[i] = true;
        }
    }
}

void GeneratorGraph::generate() {
    m_adjacencyMatrix = Matrix(m_vertexCount, m_vertexCount);

    auto degrees = m_generateDegreeSequence();

    // рёбра только от i к j, где i < j → ацикличность гарантирована
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

// ============================================================
// п.2 — эксцентриситеты, центр, диаметр
// ============================================================

void GeneratorGraph::computeEccentricities() {
    const double INF = std::numeric_limits<double>::infinity();

    // строим матрицу расстояний по неориентированному варианту
    Matrix dist(m_vertexCount, m_vertexCount, INF);

    for (int i = 0; i < m_vertexCount; ++i) {
        dist(i, i) = 0;
    }

    for (int i = 0; i < m_vertexCount; ++i) {
        for (int j = 0; j < m_vertexCount; ++j) {
            if (m_adjacencyMatrix(i, j) != 0 || m_adjacencyMatrix(j, i) != 0) {
                dist(i, j) = 1;
                dist(j, i) = 1;
            }
        }
    }

    // Флойд-Уоршалл
    for (int k = 0; k < m_vertexCount; ++k) {
        for (int i = 0; i < m_vertexCount; ++i) {
            for (int j = 0; j < m_vertexCount; ++j) {
                if (dist(i, k) + dist(k, j) < dist(i, j)) {
                    dist(i, j) = dist(i, k) + dist(k, j);
                }
            }
        }
    }

    // эксцентриситет = max расстояние от вершины
    m_eccentricities.resize(m_vertexCount);
    for (int i = 0; i < m_vertexCount; ++i) {
        double maxDist = 0;
        for (int j = 0; j < m_vertexCount; ++j) {
            if (i != j) maxDist = std::max(maxDist, dist(i, j));
        }
        m_eccentricities[i] = maxDist;
    }

    // диаметр = max эксцентриситет
    m_diameter = static_cast<int>(
        *std::max_element(m_eccentricities.begin(), m_eccentricities.end()));

    // радиус = min эксцентриситет
    double radius = *std::min_element(m_eccentricities.begin(), m_eccentricities.end());

    // центр = вершины с эксцентриситетом == радиус
    // диаметральные = вершины с эксцентриситетом == диаметр
    m_centerVertices.clear();
    m_diametralVertices.clear();
    for (int i = 0; i < m_vertexCount; ++i) {
        if (m_eccentricities[i] == radius) {
            m_centerVertices.push_back(i);
        }
        if (m_eccentricities[i] == m_diameter) {
            m_diametralVertices.push_back(i);
        }
    }
}

std::vector<double> GeneratorGraph::getEccentricities() const { return m_eccentricities; }
std::vector<int> GeneratorGraph::getCenterVertices() const { return m_centerVertices; }
std::vector<int> GeneratorGraph::getDiametralVertices() const { return m_diametralVertices; }
int GeneratorGraph::getDiameter() const { return m_diameter; }

// ============================================================
// п.3 — весовая матрица + Шимбелл
// ============================================================

void GeneratorGraph::generateWeightMatrix(WeightMode mode) {
    m_weightMatrix = Matrix(m_vertexCount, m_vertexCount);

    std::uniform_real_distribution<double> posDist(1.0, 100.0);
    std::uniform_real_distribution<double> negDist(-100.0, -1.0);
    std::uniform_real_distribution<double> mixDist(-100.0, 100.0);

    for (int i = 0; i < m_vertexCount; ++i) {
        for (int j = 0; j < m_vertexCount; ++j) {
            if (m_adjacencyMatrix(i, j) != 0) {
                switch (mode) {
                    case WeightMode::Positive:
                        m_weightMatrix(i, j) = posDist(m_rng);
                        break;
                    case WeightMode::Negative:
                        m_weightMatrix(i, j) = negDist(m_rng);
                        break;
                    case WeightMode::Mixed:
                        m_weightMatrix(i, j) = mixDist(m_rng);
                        break;
                }
            }
        }
    }
}

Matrix GeneratorGraph::shimbell(int steps, bool findMin) const {
    Matrix result = m_weightMatrix;
    for (int s = 1; s < steps; ++s) {
        result = findMin
            ? result.shimbellMin(m_weightMatrix)
            : result.shimbellMax(m_weightMatrix);
    }
    return result;
}

// ============================================================
// п.4 — маршруты
// ============================================================

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
    // DAG: топологический порядок = порядок индексов
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
