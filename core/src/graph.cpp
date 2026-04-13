#include "graph.h"

Graph::Graph(int vertexCount)
    : m_vertexCount(vertexCount)
    , m_adjacencyMatrix(vertexCount, vertexCount)
    , m_weightMatrix(vertexCount, vertexCount)
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

void Graph::printAdjacencyMatrix() const {
    std::cout << "Adjacency matrix:\n";
    m_adjacencyMatrix.print();
}

void Graph::printWeightMatrix() const {
    std::cout << "Weight matrix:\n";
    m_weightMatrix.print();
}
