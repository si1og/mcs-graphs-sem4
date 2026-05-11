#include "generator_graph.h"

GeneratorGraph::GeneratorGraph(int vertexCount)
    : Graph(vertexCount),
      m_rng(std::random_device{}()),
      m_uniformDist(0, 1),
      m_diameter(0),
      m_capacityMatrix(vertexCount, vertexCount, 0),
      m_costMatrix(vertexCount, vertexCount, 0)
{}

double GeneratorGraph::m_sampleWeibull(const WeibullParams& parameters) {
double r;
do {
    r = m_uniformDist(m_rng);
} while (r >= 1);

return parameters.shift + parameters.scale * std::pow(-std::log(1 - r), 1 / parameters.shape);
}

std::vector<int> GeneratorGraph::m_generateDegreeSequence() {
    std::vector<int> degrees(m_vertexCount);

    bool valid;
    do {
        valid = true;
        for (int i = 0; i < m_vertexCount; ++i) {
            double raw = m_sampleWeibull(constants::graph);
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
    const double INF = std::numeric_limits<double>::infinity();

    m_weightMatrix = Matrix(m_vertexCount, m_vertexCount, INF);

    std::uniform_int_distribution<int> signDist(0, 1);

    for (int i = 0; i < m_vertexCount; ++i) {
        m_weightMatrix(i, i) = 0;

        for (int j = 0; j < m_vertexCount; ++j) {
            if (i == j) continue;

            if (m_adjacencyMatrix(i, j) != 0) {
                const int WEIGHT_MATRIX_DIST_VALUE = 10;
                double w = std::round(m_sampleWeibull(constants::weight ) * WEIGHT_MATRIX_DIST_VALUE);

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

Matrix GeneratorGraph::shimbell(int steps, bool findMin) const {
    const double NO_EDGE = findMin
        ? std::numeric_limits<double>::infinity()
        : -std::numeric_limits<double>::infinity();

    Matrix W(m_vertexCount, m_vertexCount, NO_EDGE);

    if (steps == 0) {
        for (int i = 0; i < m_vertexCount; ++i) {
            W(i, i) = 0;
        }
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

    for (int i = 0; i < m_vertexCount; ++i) {
        result(i, i) = 0;
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


void GeneratorGraph::m_dfsArticulation(int v,
                                        int parent,
                                        std::vector<int>& tin,
                                        std::vector<int>& low,
                                        std::vector<bool>& visited,
                                        std::vector<bool>& isAP,
                                        int& timer,
                                        int& iterations) const {
    visited[v] = true;
    tin[v] = low[v] = timer++;

    int children = 0;

    for (int u = 0; u < m_vertexCount; ++u) {
        iterations++;

        if (u == v) continue;

        if (m_adjacencyMatrix(v, u) == 0 && m_adjacencyMatrix(u, v) == 0) {
            continue;
        }

        if (u == parent) {
            continue;
        }

        if (visited[u]) {
            low[v] = std::min(low[v], tin[u]);
        } else {
            ++children;

            m_dfsArticulation(u, v, tin, low, visited, isAP, timer, iterations);

            low[v] = std::min(low[v], low[u]);

            if (parent != -1 && low[u] >= tin[v]) {
                isAP[v] = true;
            }
        }
    }

    if (parent == -1 && children > 1) {
        isAP[v] = true;
    }
}

// упрощенный упрощённый алгоритм Тарьяна (лек. 4 стр. 11 - полный)
// не использует стек рёбер
// квадратичная сложность из-за использования матрицы смежности, со списками сложность O(V + E)
ArticulationPointsResult GeneratorGraph::findArticulationPoints() const {
    std::vector<int> tin(m_vertexCount, -1);
    std::vector<int> low(m_vertexCount, -1);
    std::vector<bool> visited(m_vertexCount, false);
    std::vector<bool> isAP(m_vertexCount, false);

    int timer = 0;
    ArticulationPointsResult result;

    for (int v = 0; v < m_vertexCount; ++v) {
        if (!visited[v]) {
            m_dfsArticulation(v, -1, tin, low, visited, isAP, timer, result.iterations);
        }
    }

    for (int v = 0; v < m_vertexCount; ++v) {
        if (isAP[v]) {
            result.points.push_back(v);
        }
    }

    return result;
}

std::vector<int> GeneratorGraph::m_restorePath(int s,
                                               int t,
                                               const std::vector<int>& H) const {
    std::vector<int> path;

    for (int v = t; v != -1; v = H[v]) {
        path.push_back(v);

        if (v == s) {
            break;
        }
    }

    std::reverse(path.begin(), path.end());

    if (path.empty() || path.front() != s) {
        path.clear();
    }

    return path;
}

//TODO: обосновать очередь
// 1) обычная очередь работает менее эффективно, т.к. мы можем сделать много лишних циклов, извлекая вершины из очереди
//    и ища оптимальную (в обычной очереди вершины расположены в порядке добавления в очередь, а не отсортированы по весам)
//  - заменил на очередь c приоритетами (как написано на слайде)
//
// 2) кол-во итераций всегда выводится
//  - теперь выводится всегда
//
// 3) при перегенерации графа вводить число вершин
//  - поправил
ShortestPathResult GeneratorGraph::dijkstraNegative(
    int s,
    int t,
    const Matrix& adjacency,
    const Matrix& weight
) const {
    const double INF = std::numeric_limits<double>::infinity();

    ShortestPathResult result;

    // T[v] - длина клатчайщего пути от s к v
    // H[v] - вершина, предшествующая v по кратчайшему пути

    std::vector<double> T(m_vertexCount, INF);
    std::vector<int> H(m_vertexCount, -1);

    std::vector<int> relaxCount(m_vertexCount, 0);

    // { T[v], v }
    using QueueItem = std::pair<double, int>;

    // очередь с приоритетами
    // сначала извлекается вершина с минимальным T[v]
    std::priority_queue<
        QueueItem,
        std::vector<QueueItem>,
        std::greater<QueueItem>
    > Q;

    int iterations = 0;

    T[s] = 0;
    H[s] = -1;

    Q.push({0, s});

    while (!Q.empty()) {
        double currentDistance = Q.top().first;
        int v = Q.top().second;

        Q.pop();

        // пропускаем устаревшую запись
        if (currentDistance != T[v]) {
            continue;
        }

        for (int u = 0; u < m_vertexCount; ++u) {
            ++iterations;

            if (adjacency(v, u) == 0) {
                continue;
            }

            // по нер-ву треугольника
            if (T[v] != INF && T[u] > T[v] + weight(v, u)) {
                T[u] = T[v] + weight(v, u);
                H[u] = v;

                Q.push({T[u], u});

                ++relaxCount[u];

                // выходим, если пошли по циклу
                if (relaxCount[u] >= m_vertexCount) {
                    result.hasNegativeCycle = true;
                    break;
                }
            }
        }

        if (result.hasNegativeCycle) {
            break;
        }
    }

    result.T = T;
    result.H = H;

    result.iterations = iterations;

    result.hasPath = T[t] != INF;
    result.distance = T[t];

    if (result.hasPath && !result.hasNegativeCycle) {
        result.path = m_restorePath(s, t, H);
    }

    return result;
}

ShortestPathResult GeneratorGraph::dijkstraNegative(int s, int t) const {
    return dijkstraNegative(
        s,
        t,
        m_adjacencyMatrix,
        m_weightMatrix
    );
}

// lab3

void GeneratorGraph::generateCapacityAndCostMatrices() {
    m_capacityMatrix = Matrix(m_vertexCount, m_vertexCount, 0);
    m_costMatrix = Matrix(m_vertexCount, m_vertexCount, 0);

    for (int i = 0; i < m_vertexCount; ++i) {
        for (int j = 0; j < m_vertexCount; ++j) {
            if (m_adjacencyMatrix(i, j) != 0) {
                double capacityRaw = m_sampleWeibull(constants::capacity);
                double costRaw = m_sampleWeibull(constants::cost);

                int capacity = std::max(1, static_cast<int>(std::round(capacityRaw * 10)));
                int cost = std::max(1, static_cast<int>(std::round(costRaw * 10)));

                m_capacityMatrix(i, j) = capacity;
                m_costMatrix(i, j) = cost;
            }
        }
    }

    isMatrixInit.capacity = true;
    isMatrixInit.cost = true;
}

const Matrix& GeneratorGraph::getCapacityMatrix() const {
    return m_capacityMatrix;
}

const Matrix& GeneratorGraph::getCostMatrix() const {
    return m_costMatrix;
}

void GeneratorGraph::printCapacityMatrix() const {
    std::cout << "Матрица пропускных способностей:\n";
    m_capacityMatrix.print();
}

void GeneratorGraph::printCostMatrix() const {
    std::cout << "Матрица стоимостей:\n";
    m_costMatrix.print();
}

bool GeneratorGraph::isCapacityMatrixGenerated() const {
    return isMatrixInit.capacity;
}

bool GeneratorGraph::isCostMatrixGenerated() const {
    return isMatrixInit.cost;
}

// алгоритм Форда-Фалкерсона
// реализован через расстановку пометок
MaxFlowResult GeneratorGraph::fordFulkerson(int source, int sink) const {
    const int INF = std::numeric_limits<int>::max();

    MaxFlowResult result(m_vertexCount);

    if (!isMatrixInit.capacity) {
        return result;
    }

    if (source < 0 || source >= m_vertexCount ||
        sink < 0 || sink >= m_vertexCount ||
        source == sink) {
        return result;
    }

    struct Mark {
        char sign = '+';
        int parent = -1;
        int delta = 0;
    };

    while (true) {
        // S[v] - вершина получила пометку
        // N[v] - вершина уже обработана
        // P[v] - пометка вершины: знак, предок, величина изменения потока
        std::vector<bool> S(m_vertexCount, false);
        std::vector<bool> N(m_vertexCount, false);
        std::vector<Mark> P(m_vertexCount);

        S[source] = true;
        P[source] = {'+', -1, INF};

        bool expanded = true;

        // расставляем пометки, пока множество S расширяется
        while (expanded && !S[sink]) {
            expanded = false;

            for (int v = 0; v < m_vertexCount; ++v) {
                ++result.iterations;

                if (!S[v] || N[v]) {
                    continue;
                }

                // прямые дуги
                for (int u = 0; u < m_vertexCount; ++u) {
                    ++result.iterations;

                    if (S[u]) {
                        continue;
                    }

                    if (m_capacityMatrix(v, u) <= 0) {
                        continue;
                    }

                    int available =
                        static_cast<int>(m_capacityMatrix(v, u) - result.flowMatrix(v, u));

                    if (available > 0) {
                        S[u] = true;
                        P[u] = {
                            '+',
                            v,
                            std::min(P[v].delta, available)
                        };

                        expanded = true;

                        if (u == sink) {
                            break;
                        }
                    }
                }

                if (S[sink]) {
                    break;
                }

                // обратные дуги
                for (int u = 0; u < m_vertexCount; ++u) {
                    ++result.iterations;

                    if (S[u]) {
                        continue;
                    }

                    if (result.flowMatrix(u, v) > 0) {
                        int available =
                            static_cast<int>(result.flowMatrix(u, v));

                        S[u] = true;
                        P[u] = {
                            '-',
                            v,
                            std::min(P[v].delta, available)
                        };

                        expanded = true;

                        if (u == sink) {
                            break;
                        }
                    }
                }

                N[v] = true;

                if (S[sink]) {
                    break;
                }
            }
        }

        // если сток не получил пометку, увеличивающих цепей больше нет
        if (!S[sink]) {
            break;
        }

        int delta = P[sink].delta;
        int current = sink;

        // изменяем поток по найденной увеличивающей цепи
        while (current != source) {
            Mark mark = P[current];
            int previous = mark.parent;

            if (mark.sign == '+') {
                result.flowMatrix(previous, current) += delta;
            } else {
                result.flowMatrix(current, previous) -= delta;
            }

            current = previous;
        }

        result.maxFlow += delta;
    }

    return result;
}

// поток минимальной стоимости
// заданная величина потока передаётся параметром requiredFlow
// на каждой итерации ищем кратчайший путь в остаточной сети
MinCostFlowResult GeneratorGraph::minCostFlow(int source,
                                              int sink,
                                              int requiredFlow) const {
    MinCostFlowResult result(m_vertexCount);

    result.requiredFlow = requiredFlow;

    if (!isMatrixInit.capacity || !isMatrixInit.cost) {
        return result;
    }

    if (source < 0 || source >= m_vertexCount ||
        sink < 0 || sink >= m_vertexCount ||
        source == sink ||
        requiredFlow <= 0) {
        return result;
    }

    Matrix residualCapacity = m_capacityMatrix;
    Matrix residualCost(m_vertexCount, m_vertexCount, 0);

    for (int i = 0; i < m_vertexCount; ++i) {
        for (int j = 0; j < m_vertexCount; ++j) {
            if (m_capacityMatrix(i, j) > 0) {
                residualCost(i, j) = m_costMatrix(i, j);
                residualCost(j, i) = -m_costMatrix(i, j);
            }
        }
    }

    while (result.achievedFlow < requiredFlow) {
        Matrix residualAdjacency(m_vertexCount, m_vertexCount, 0);

        for (int i = 0; i < m_vertexCount; ++i) {
            for (int j = 0; j < m_vertexCount; ++j) {
                if (residualCapacity(i, j) > 0) {
                    residualAdjacency(i, j) = 1;
                }
            }
        }

        auto pathResult = dijkstraNegative(
            source,
            sink,
            residualAdjacency,
            residualCost
        );

        result.iterations += pathResult.iterations;

        if (!pathResult.hasPath || pathResult.hasNegativeCycle) {
            break;
        }

        int addFlow = requiredFlow - result.achievedFlow;

        for (size_t i = 1; i < pathResult.path.size(); ++i) {
            int from = pathResult.path[i - 1];
            int to = pathResult.path[i];

            addFlow = std::min(
                addFlow,
                static_cast<int>(residualCapacity(from, to))
            );
        }

        if (addFlow <= 0) {
            break;
        }

        for (size_t i = 1; i < pathResult.path.size(); ++i) {
            int from = pathResult.path[i - 1];
            int to = pathResult.path[i];

            residualCapacity(from, to) -= addFlow;
            residualCapacity(to, from) += addFlow;

            if (m_capacityMatrix(from, to) > 0) {
                // идём по исходной дуге
                result.flowMatrix(from, to) += addFlow;
                result.totalCost += addFlow * static_cast<int>(m_costMatrix(from, to));
            } else {
                // идём по обратной дуге остаточной сети
                // значит уменьшаем поток по исходной дуге to -> from
                result.flowMatrix(to, from) -= addFlow;
                result.totalCost -= addFlow * static_cast<int>(m_costMatrix(to, from));
            }
        }

        result.achievedFlow += addFlow;
    }

    result.success = result.achievedFlow == requiredFlow;

    return result;
}
