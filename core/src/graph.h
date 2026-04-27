#pragma once

#include "matrix.h"

class Graph {
public:
    explicit Graph(int vertexCount);
    virtual ~Graph() = default;

    int getVertexCount() const;
    const Matrix& getAdjacencyMatrix() const;
    const Matrix& getWeightMatrix() const;

    void printAdjacencyMatrix() const;
    void printWeightMatrix() const;

    struct isMatrixInit {
        bool adjacency = false;
        bool weight = false;
    } isMatrixInit;

protected:
    int m_vertexCount;
    Matrix m_adjacencyMatrix;
    Matrix m_weightMatrix;

    void m_resetAdjacencyMatrix();
    void m_resetWeightMatrix();
};
