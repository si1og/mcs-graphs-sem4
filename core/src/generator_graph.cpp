#include "generator_graph.h"
#include <cmath>
#include <set>
#include <stack>
#include <stdexcept>
#include <vector>

namespace {
bool matricesEqual(const Matrix& lhs, const Matrix& rhs) {
    if (lhs.rows() != rhs.rows() || lhs.cols() != rhs.cols()) {
        return false;
    }

    for (int i = 0; i < lhs.rows(); ++i) {
        for (int j = 0; j < lhs.cols(); ++j) {
            const double left = lhs(i, j);
            const double right = rhs(i, j);

            if (std::isinf(left) || std::isinf(right)) {
                if (!(std::isinf(left) && std::isinf(right))) {
                    return false;
                }
            } else if (std::abs(left - right) > 1e-9) {
                return false;
            }
        }
    }

    return true;
}
}

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

    m_syncUndirectedAdjacencyMatrix();
    isMatrixInit.adjacency = true;
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
    m_syncUndirectedWeightMatrix();
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
// TODO:
// 2) кол-во итераций всегда выводится
//  - теперь выводится всегда
//
// TODO:
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

ShortestPathResult GeneratorGraph::dijkstraNegative(int s, int t, const Matrix& adjacency) const {
    return dijkstraNegative(
        s,
        t,
        adjacency,
        adjacency
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

    for (int i = 0; i < m_vertexCount; ++i) {
        for (int j = 0; j < m_vertexCount; ++j) {
            result.costFlowMatrix(i, j) =
                m_costMatrix(i, j) * result.flowMatrix(i, j);
        }
    }

    return result;
}

// lab4
SpanningTreesResult GeneratorGraph::countSpanningTreesKirchhoff() const {
    SpanningTreesResult result(m_vertexCount);
    const Matrix& adjacency = m_undirectedAdjacencyMatrix;

    for (int i = 0; i < m_vertexCount; ++i) {
        int degree = 0;

        for (int j = 0; j < m_vertexCount; ++j) {
            if (i == j) {
                continue;
            }

            if (adjacency(i, j) != 0) {
                ++degree;
                result.kirchhoffMatrix(i, j) = -1;
            }
        }

        result.kirchhoffMatrix(i, i) = degree;
    }

    if (m_vertexCount <= 1) {
        result.count = 1;
        return result;
    }

    for (int i = 1; i < m_vertexCount; ++i) {
        for (int j = 1; j < m_vertexCount; ++j) {
            result.cofactorMatrix(i - 1, j - 1) =
                result.kirchhoffMatrix(i, j);
        }
    }

    result.cofactorDeterminant = result.cofactorMatrix.determinant();
    result.count = std::llround(result.cofactorDeterminant);

    return result;
}

KruskalResult GeneratorGraph::m_kruskalAlgorithm() const {
    KruskalResult result(m_vertexCount);
    std::vector<WeightedEdge> edges;

    for (int i = 0; i < m_vertexCount; ++i) {
        result.treeMatrix(i, i) = 0;
    }

    // берем все ребра графа
    for (int i = 0; i < m_vertexCount; ++i) {
        for (int j = i + 1; j < m_vertexCount; ++j) {
            if (m_undirectedAdjacencyMatrix(i, j) != 0 &&
                !std::isinf(m_undirectedWeightMatrix(i, j))) {
                edges.push_back({
                    i,
                    j,
                    m_undirectedWeightMatrix(i, j)
                });
            }
        }
    }

    // сортируем рёбра по возрастанию веса
    std::sort(
        edges.begin(),
        edges.end(),
        [](const WeightedEdge& lhs, const WeightedEdge& rhs) {
            if (lhs.weight != rhs.weight) {
                return lhs.weight < rhs.weight;
            }

            if (lhs.from != rhs.from) {
                return lhs.from < rhs.from;
            }

            return lhs.to < rhs.to;
        }
    );

    std::vector<int> parent(m_vertexCount);
    std::vector<int> rank(m_vertexCount, 0);

    for (int i = 0; i < m_vertexCount; ++i) {
        parent[i] = i;
    }

    std::function<int(int)> findSet = [&](int vertex) {
        if (parent[vertex] != vertex) {
            parent[vertex] = findSet(parent[vertex]);
        }

        return parent[vertex];
    };

    auto unionSets = [&](int first, int second) {
        first = findSet(first);
        second = findSet(second);

        if (first == second) {
            return false;
        }

        if (rank[first] < rank[second]) {
            std::swap(first, second);
        }

        parent[second] = first;

        if (rank[first] == rank[second]) {
            ++rank[first];
        }

        // если ребро соединяет вершины из разных множеств, его можно взять
        return true;
    };

    // идем по ребрам от самого дешевого к самому дорогому
    // изначально каждая вершина в своем множестве [сверху как раз сделаны ф. поиска и пересечения «множеств»]
    for (const auto& edge : edges) {
        if (unionSets(edge.from, edge.to)) {
            // добавляем ребро в остов, если оно не образует цикл
            result.edges.push_back(edge);
            result.treeMatrix(edge.from, edge.to) = edge.weight;
            result.treeMatrix(edge.to, edge.from) = edge.weight;
            result.totalWeight += edge.weight;

            // останавливаемся, когда выбрали n - 1 ребро
            if (static_cast<int>(result.edges.size()) == m_vertexCount - 1) {
                break;
            }
        }
    }

    result.success =
        static_cast<int>(result.edges.size()) == m_vertexCount - 1;

    return result;
}

KruskalMinimumSpanningTreeResult GeneratorGraph::kruskalMinimumSpanningTree() const {
    KruskalMinimumSpanningTreeResult result(std::move(m_kruskalAlgorithm()), m_vertexCount);

    for (int i = 0; i < m_vertexCount; ++i) {
        result.decodedTreeMatrix(i, i) = 0;
    }

    if (!result.kruskal.success) {
        return result;
    }

    result.pruferCode = m_encodePruferCode(result.kruskal.edges);
    result.decodedTreeMatrix = m_decodePruferCode(result.pruferCode);
    result.pruferRoundTripSuccess =
        matricesEqual(result.kruskal.treeMatrix, result.decodedTreeMatrix);

    return result;
}

std::vector<PruferCodeItem> GeneratorGraph::m_encodePruferCode(
    const std::vector<WeightedEdge>& treeEdges
) const {
    // кодируем полученный остов кодом Прюфера с сохранением весов рёбер
    std::vector<PruferCodeItem> code;
    std::vector<std::set<std::pair<int, double>>> adjacency(m_vertexCount);

    for (const auto& edge : treeEdges) {
        adjacency[edge.from].insert({edge.to, edge.weight});
        adjacency[edge.to].insert({edge.from, edge.weight});
    }

    for (int step = 0; step < m_vertexCount - 1; ++step) {
        int leaf = -1;

        // на каждом шаге берём лист с минимальным номером
        for (int vertex = 0; vertex < m_vertexCount; ++vertex) {
            if (adjacency[vertex].size() == 1) {
                leaf = vertex;
                break;
            }
        }

        if (leaf == -1) {
            break;
        }

        const auto [neighbor, weight] = *adjacency[leaf].begin();
        // в код записываем соседа листа, а рядом сохраняем вес удаляемого ребра
        code.push_back({neighbor, weight});

        // удаляем лист из текущего дерева
        adjacency[neighbor].erase({leaf, weight});
        adjacency[leaf].clear();
    }

    return code;
}

Matrix GeneratorGraph::m_decodePruferCode(
    const std::vector<PruferCodeItem>& code
) const {
    // декодируем код Прюфера обратно в матрицу весов остова
    Matrix decodedTreeMatrix(
        m_vertexCount,
        m_vertexCount,
        std::numeric_limits<double>::infinity()
    );
    std::vector<int> unusedVertices(m_vertexCount);

    for (int i = 0; i < m_vertexCount; ++i) {
        decodedTreeMatrix(i, i) = 0;
        unusedVertices[i] = i;
    }

    const int steps = std::min(
        m_vertexCount - 1,
        static_cast<int>(code.size())
    );

    for (int i = 0; i < steps; ++i) {
        std::set<int> remainingCode;

        for (int j = i; j < static_cast<int>(code.size()); ++j) {
            remainingCode.insert(code[j].vertex);
        }

        // выбираем минимальную вершину, которой нет в оставшемся коде
        auto leafIt = std::find_if(
            unusedVertices.begin(),
            unusedVertices.end(),
            [&](int vertex) {
                return remainingCode.find(vertex) == remainingCode.end();
            }
        );

        if (leafIt == unusedVertices.end()) {
            break;
        }

        const int leaf = *leafIt;
        const int neighbor = code[i].vertex;
        const double weight = code[i].weight;

        // восстанавливаем ребро с тем же весом, который был сохранён при кодировании
        decodedTreeMatrix(leaf, neighbor) = weight;
        decodedTreeMatrix(neighbor, leaf) = weight;

        unusedVertices.erase(leafIt);
    }

    return decodedTreeMatrix;
}

EdmondsBlossomResult
GeneratorGraph::edmondsBlossomInOriginalGraph() const {
    const Matrix& weights = isMatrixInit.weight
        ? m_undirectedWeightMatrix
        : m_undirectedAdjacencyMatrix;

    return m_edmondsBlossom(
        m_undirectedAdjacencyMatrix,
        weights,
        isMatrixInit.weight
    );
}

EdmondsBlossomResult
GeneratorGraph::edmondsBlossomInMinimumSpanningTree() const {
    EdmondsBlossomResult result(m_vertexCount);
    auto kruskalMinimumSpanningTreeResult = kruskalMinimumSpanningTree();

    if (!kruskalMinimumSpanningTreeResult.kruskal.success) {
        result.success = false;
        return result;
    }

    Matrix adjacency(m_vertexCount, m_vertexCount, 0);

    for (const auto& edge : kruskalMinimumSpanningTreeResult.kruskal.edges) {
        adjacency(edge.from, edge.to) = 1;
        adjacency(edge.to, edge.from) = 1;
    }

    return m_edmondsBlossom(
        adjacency,
        kruskalMinimumSpanningTreeResult.kruskal.treeMatrix,
        true
    );
}

EdmondsBlossomResult GeneratorGraph::m_edmondsBlossom(
    const Matrix& adjacency,
    const Matrix& weights,
    bool hasWeights
) const {
    EdmondsBlossomResult result(m_vertexCount);
    result.hasWeights = hasWeights;

    if (hasWeights) {
        result.edgeMatrix = Matrix(
            m_vertexCount,
            m_vertexCount,
            std::numeric_limits<double>::infinity()
        );

        for (int i = 0; i < m_vertexCount; ++i) {
            result.edgeMatrix(i, i) = 0;
        }
    }

    const auto graph = m_matrixToAdjacencyList<int>(
        adjacency,
        [](int, int to) {
            return to;
        }
    );

    // хотим выбрать как можно больше рёбер, чтобы они не касались друг друга
    // => ищем увел. путь, путь от одной вершины до другой, где рёбра идут по очереди

    // BEGIN
    // а нечётные циклы временно сжимаем.
    // match[v]: с какой вершиной соединена вершина v
    std::vector<int> match(m_vertexCount, -1);
    // parent[v]: предыдущая вершина в дереве поиска увеличивающегося пути
    std::vector<int> parent(m_vertexCount);
    std::vector<int> base(m_vertexCount);
    // помечаем, что вершина уже добавлена в очередь
    std::vector<bool> used(m_vertexCount);
    // помечаем цикл в графе
    std::vector<bool> blossom(m_vertexCount);
    std::queue<int> queue;

    // ищем общую базу нечётного цикла, чтобы сжать к одной вершине
    auto findLca = [&](int first, int second) {
        std::vector<bool> usedPath(m_vertexCount, false);

        while (true) {
            first = base[first];
            usedPath[first] = true;

            if (match[first] == -1) {
                break;
            }

            first = parent[match[first]];
        }

        while (true) {
            second = base[second];

            if (usedPath[second]) {
                return second;
            }

            second = parent[match[second]];
        }
    };

    auto markPath = [&](int vertex,
                        int lca,
                        int child,
                        auto&& markPathRef) -> void {
        // помечаем путь от вершины до общей базы
        while (base[vertex] != lca) {
            blossom[base[vertex]] = true;
            blossom[base[match[vertex]]] = true;
            parent[vertex] = child;
            child = match[vertex];
            vertex = parent[match[vertex]];
        }
    };

    auto findPath = [&](int root) {
        // T <- ø: очищаем BFS-дерево поиска увеличивающего пути
        std::fill(used.begin(), used.end(), false);
        std::fill(parent.begin(), parent.end(), -1);

        for (int i = 0; i < m_vertexCount; ++i) {
            base[i] = i;
        }

        while (!queue.empty()) {
            queue.pop();
        }

        // pick r in F; queue.push(r); T.add(r)
        queue.push(root);
        used[root] = true;

        // WHILE queue != ø
        while (!queue.empty()) {
            // v <- queue.pop()
            int vertex = queue.front();
            queue.pop();

            // FOR ALL neighbors w of v DO
            for (int to : graph[vertex]) {
                if (base[vertex] == base[to] ||
                    match[vertex] == to) {
                    continue;
                }

                if (to == root ||
                    (match[to] != -1 && parent[match[to]] != -1)) {
                    // ELSE IF w in T AND odd-length cycle detected THEN
                    int lca = findLca(vertex, to);
                    std::fill(blossom.begin(), blossom.end(), false);

                    // contract cycle: найден нечётный цикл, сжимаем blossom к общей базе
                    markPath(vertex, lca, to, markPath);
                    markPath(to, lca, vertex, markPath);

                    for (int i = 0; i < m_vertexCount; ++i) {
                        if (blossom[base[i]]) {
                            base[i] = lca;

                            if (!used[i]) {
                                used[i] = true;
                                queue.push(i);
                            }
                        }
                    }
                } else if (parent[to] == -1) {
                    // IF w not in T AND w matched THEN T.add(w), T.add(mate(w))
                    parent[to] = vertex;

                    if (match[to] == -1) {
                        // ELSE IF w in F THEN свободная вершина завершает увеличивающий путь
                        return to;
                    }

                    // queue.push(mate(w))
                    int next = match[to];
                    used[next] = true;
                    queue.push(next);
                }
            }
        }

        return -1;
    };

    auto augment = [&](int vertex) {
        // expand all contracted nodes; reconstruct augmenting path;
        // invert augmenting path
        while (vertex != -1) {
            int previous = parent[vertex];
            int next = (previous == -1) ? -1 : match[previous];

            match[vertex] = previous;

            if (previous != -1) {
                match[previous] = vertex;
            }

            vertex = next;
        }
    };

    // WHILE F != ø DO: пробуем стартовать из каждой свободной вершины
    for (int vertex = 0; vertex < m_vertexCount; ++vertex) {
        if (match[vertex] == -1) {
            int endpoint = findPath(vertex);

            if (endpoint != -1) {
                augment(endpoint);
            }
        }
    }
    // END

    for (int from = 0; from < m_vertexCount; ++from) {
        int to = match[from];

        if (to == -1 || from > to) {
            continue;
        }

        double value = hasWeights ? weights(from, to) : 1;

        result.edges.push_back({from, to, value});
        result.edgeMatrix(from, to) = value;
        result.edgeMatrix(to, from) = value;
    }

    return result;
}

// lab5
// Алгоритм приведения графа к эйлерову виду:
// 1. Найти все вершины нечётной степени.
// 2. Пока существуют нечётные вершины:
//     * выбрать две вершины нечётной степени (u) и (v);
//     * если ребро ((u,v)) существует, удалить его;
//     * иначе добавить ребро ((u,v)).
// Патч для сохранения связности: перебираем пары нечётных вершин и перед
// удалением ребра проверяем наличие обходного пути. Мост удаляется только
// если пользователь явно разрешил получить несвязный результат.
// После каждой такой операции степени вершин (u) и (v) меняют чётность, поэтому число вершин нечётной степени уменьшается на 2. Поскольку количество нечётных вершин в графе всегда чётно, процесс завершится за конечное число шагов.

CheckIfEulerianGraphResult GeneratorGraph::checkIfEulerianGraph(
    bool allowDisconnectedResult
) const {
    bool isEulerian = true;
    bool transformationCompleted = true;
    bool resultIsConnected = true;
    Matrix adjacency = getUndirectedAdjacencyMatrix();
    std::vector<Edge> addedEdges;
    std::vector<Edge> removedEdges;
    auto getVertexDegree = [&](size_t i) {
        int degree = 0;
        for (size_t j = 0; j < adjacency.cols(); ++j) {
            if (i == j) continue;

            if (adjacency(i, j) != 0) ++degree;
        }

        return degree;
    };

    std::vector<int> verticesWithOddDegrees;

    while (true) {
        verticesWithOddDegrees.clear();

        for (size_t i = 0; i < adjacency.rows(); ++i) {
            if (getVertexDegree(i) % 2 != 0) {
                isEulerian = false;
                verticesWithOddDegrees.push_back(i);
            }
        }

        if (verticesWithOddDegrees.empty()) {
            break;
        }

        bool graphChanged = false;

        for (size_t i = 0; i < verticesWithOddDegrees.size() && !graphChanged; ++i) {
            for (size_t j = i + 1; j < verticesWithOddDegrees.size(); ++j) {
                const auto u = verticesWithOddDegrees[i];
                const auto v = verticesWithOddDegrees[j];

                if (adjacency(u, v) == 0) {
                    adjacency(u, v) = 1;
                    adjacency(v, u) = 1;
                    addedEdges.emplace_back(u, v);
                    graphChanged = true;
                    break;
                }

                Matrix temp = adjacency;
                temp(u, v) = 0;
                temp(v, u) = 0;

                const auto resultPath = dijkstraNegative(u, v, temp);

                if (resultPath.hasPath || allowDisconnectedResult) {
                    adjacency(u, v) = 0;
                    adjacency(v, u) = 0;
                    removedEdges.emplace_back(u, v);

                    if (!resultPath.hasPath) {
                        resultIsConnected = false;
                    }

                    graphChanged = true;
                    break;
                }
            }
        }

        if (!graphChanged) {
            transformationCompleted = false;
            break;
        }
    }

    FleuryResult eulerianCycle;

    if (transformationCompleted && resultIsConnected) {
        eulerianCycle = m_flueryAlgorithm(adjacency);
    }

    return CheckIfEulerianGraphResult(
        isEulerian,
        transformationCompleted,
        std::move(addedEdges),
        std::move(removedEdges),
        std::move(eulerianCycle),
        std::move(adjacency)
    );
}

FleuryResult GeneratorGraph::m_flueryAlgorithm(const Matrix& adjacency) const {
    auto graph = m_matrixToAdjacencyList<int>(
        adjacency,
        [](int, int to) {
            return to;
        }
    );

    FleuryResult path;
    std::stack<int> stack;
    stack.push(0);

    while (!stack.empty()) {
        const int node = stack.top();

        if (graph[node].empty()) {
            path.emplace_back(node);
            stack.pop();

        } else {
            const int next = graph[node].front();
            graph[node].erase(graph[node].begin());

            const auto reverseEdge = std::find(
                graph[next].begin(),
                graph[next].end(),
                node
            );

            if (reverseEdge == graph[next].end()) {
                throw std::logic_error(
                    "Обратное ребро отсутствует в списке смежности"
                );
            }

            graph[next].erase(reverseEdge);
            stack.push(next);
        }
    }

    std::reverse(path.begin(), path.end());
    return path;
}

FundamentalCutsResult GeneratorGraph::buildFundamentalCutSystem() const {
    FundamentalCutsResult result(m_vertexCount);

    if (!isMatrixInit.adjacency || !isMatrixInit.weight) {
        return result;
    }

    result.spanningTree = m_kruskalAlgorithm();

    if (!result.spanningTree.success) {
        return result;
    }

    AdjacencyList tree(m_vertexCount);

    for (const auto& edge : result.spanningTree.edges) {
        tree[edge.from].push_back(edge.to);
        tree[edge.to].push_back(edge.from);
    }

    for (const auto& removedEdge : result.spanningTree.edges) {
        std::vector<bool> firstComponent(m_vertexCount, false);
        std::vector<int> stack = {removedEdge.from};
        firstComponent[removedEdge.from] = true;

        // Удаление ребра остова делит дерево на две компоненты.
        while (!stack.empty()) {
            const int vertex = stack.back();
            stack.pop_back();

            for (int to : tree[vertex]) {
                const bool isRemovedEdge =
                    (vertex == removedEdge.from && to == removedEdge.to) ||
                    (vertex == removedEdge.to && to == removedEdge.from);

                if (!isRemovedEdge && !firstComponent[to]) {
                    firstComponent[to] = true;
                    stack.push_back(to);
                }
            }
        }

        GraphCut cut;

        // В разрез входят все рёбра исходного графа между компонентами.
        for (int from = 0; from < m_vertexCount; ++from) {
            for (int to = from + 1; to < m_vertexCount; ++to) {
                if (m_undirectedAdjacencyMatrix(from, to) != 0 &&
                    firstComponent[from] != firstComponent[to]) {
                    cut.emplace(from, to);
                }
            }
        }

        result.fundamentalCuts.push_back({
            {removedEdge.from, removedEdge.to},
            std::move(cut)
        });
    }

    result.success =
        static_cast<int>(result.fundamentalCuts.size()) == m_vertexCount - 1;
    return result;
}

GraphCut GeneratorGraph::symmetricDifferenceOfFundamentalCuts(
    const FundamentalCutsResult& system,
    const std::vector<int>& selectedCutIndices
) const {
    GraphCut result;

    for (int index : selectedCutIndices) {
        if (index < 0 || index >= static_cast<int>(system.fundamentalCuts.size())) {
            throw std::out_of_range("Номер фундаментального разреза вне диапазона");
        }

        result = m_symmetricDifference(
            result,
            system.fundamentalCuts[index].cut
        );
    }

    return result;
}

// этот метод показывает, что используем метод из лекции для вычисления симм. разности
// можно найти на слайде 18 «Подпространства циклов и коциклов»
GraphCut GeneratorGraph::m_symmetricDifference(
    const GraphCut& first,
    const GraphCut& second
) const {
    GraphCut result;
    GraphCut edgeCoordinates = first;
    edgeCoordinates.insert(second.begin(), second.end());

    // разрез рассматриваем как вектор векторного пространства, натянутого на множество рёбер
    // коэффициент равен 1, если ребро входит в разрез, иначе равен 0
    for (const auto& edge : edgeCoordinates) {
        const int firstCoefficient = first.count(edge) == 0 ? 0 : 1;
        const int secondCoefficient = second.count(edge) == 0 ? 0 : 1;
        const int resultCoefficient =
            (firstCoefficient + secondCoefficient) % 2;

        if (resultCoefficient == 1) {
            result.insert(edge);
        }
    }

    return result;
}
