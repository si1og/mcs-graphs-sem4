#include "graph.h"
#include <algorithm>
#include <cmath>
#include <limits>

Graph::Graph(int vertexCount)
    : m_vertexCount(vertexCount)
    , m_adjacencyMatrix(vertexCount, vertexCount)
    , m_weightMatrix(vertexCount, vertexCount)
    , m_undirectedAdjacencyMatrix(vertexCount, vertexCount)
    , m_undirectedWeightMatrix(vertexCount, vertexCount)
{}

int Graph::getVertexCount() const {
    return m_vertexCount;
}

const Matrix& Graph::getAdjacencyMatrix() const {
    return m_adjacencyMatrix;
}

const Matrix& Graph::getWeightMatrix() const {
    return m_weightMatrix;
}

const Matrix& Graph::getUndirectedAdjacencyMatrix() const {
    return m_undirectedAdjacencyMatrix;
}

const Matrix& Graph::getUndirectedWeightMatrix() const {
    return m_undirectedWeightMatrix;
}

AdjacencyList Graph::getAdjacencyList() const {
    return m_matrixToAdjacencyList<int>(
        m_adjacencyMatrix,
        [](int, int to) {
            return to;
        }
    );
}

WeightedAdjacencyList Graph::getWeightedAdjacencyList() const {
    return m_matrixToAdjacencyList<WeightedNeighbor>(
        m_adjacencyMatrix,
        [&](int from, int to) {
            return WeightedNeighbor{to, m_weightMatrix(from, to)};
        }
    );
}

void Graph::printAdjacencyMatrix() const {
    std::cout << "Матрица смежности:\n";
    m_adjacencyMatrix.print();
}

void Graph::printWeightMatrix() const {
    std::cout << "Матрица весов:\n";
    m_weightMatrix.print();
}

void Graph::printUndirectedAdjacencyMatrix() const {
    std::cout << "Матрица смежности неориентированного графа:\n";
    m_undirectedAdjacencyMatrix.print();
}

void Graph::printUndirectedWeightMatrix() const {
    std::cout << "Матрица весов неориентированного графа:\n";
    m_undirectedWeightMatrix.print();
}

void Graph::m_resetAdjacencyMatrix() {
    m_adjacencyMatrix = Matrix(m_vertexCount, m_vertexCount);
    m_undirectedAdjacencyMatrix = Matrix(m_vertexCount, m_vertexCount);
    isMatrixInit.adjacency = false;
}

void Graph::m_resetWeightMatrix() {
    m_weightMatrix = Matrix(m_vertexCount, m_vertexCount);
    m_undirectedWeightMatrix = Matrix(m_vertexCount, m_vertexCount);
    isMatrixInit.weight = false;
}

void Graph::m_syncUndirectedAdjacencyMatrix() {
    m_undirectedAdjacencyMatrix = Matrix(m_vertexCount, m_vertexCount);

    for (int i = 0; i < m_vertexCount; ++i) {
        for (int j = i + 1; j < m_vertexCount; ++j) {
            if (m_adjacencyMatrix(i, j) != 0 ||
                m_adjacencyMatrix(j, i) != 0) {
                m_undirectedAdjacencyMatrix(i, j) = 1;
                m_undirectedAdjacencyMatrix(j, i) = 1;
            }
        }
    }
}

void Graph::m_syncUndirectedWeightMatrix() {
    const double INF = std::numeric_limits<double>::infinity();

    m_undirectedWeightMatrix = Matrix(m_vertexCount, m_vertexCount, INF);

    for (int i = 0; i < m_vertexCount; ++i) {
        m_undirectedWeightMatrix(i, i) = 0;
    }

    for (int i = 0; i < m_vertexCount; ++i) {
        for (int j = i + 1; j < m_vertexCount; ++j) {
            double weight = INF;

            if (!std::isinf(m_weightMatrix(i, j))) {
                weight = m_weightMatrix(i, j);
            }

            if (!std::isinf(m_weightMatrix(j, i))) {
                weight = std::min(weight, m_weightMatrix(j, i));
            }

            if (!std::isinf(weight)) {
                m_undirectedWeightMatrix(i, j) = weight;
                m_undirectedWeightMatrix(j, i) = weight;
            }
        }
    }
}
