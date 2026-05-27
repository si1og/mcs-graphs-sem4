#pragma once

#include "matrix.h"

class Graph {
public:
    explicit Graph(int vertexCount);
    virtual ~Graph() = default;

    int getVertexCount() const;
    const Matrix& getAdjacencyMatrix() const;
    const Matrix& getWeightMatrix() const;
    const Matrix& getUndirectedAdjacencyMatrix() const;
    const Matrix& getUndirectedWeightMatrix() const;

    void printAdjacencyMatrix() const;
    void printWeightMatrix() const;
    void printUndirectedAdjacencyMatrix() const;
    void printUndirectedWeightMatrix() const;

    struct isMatrixInit {
        bool adjacency = false;
        bool weight = false;
        bool capacity = false;
        bool cost = false;
    } isMatrixInit;

protected:
    int m_vertexCount;
    Matrix m_adjacencyMatrix;
    Matrix m_weightMatrix;
    Matrix m_undirectedAdjacencyMatrix;
    Matrix m_undirectedWeightMatrix;

    void m_resetAdjacencyMatrix();
    void m_resetWeightMatrix();
    void m_syncUndirectedAdjacencyMatrix();
    void m_syncUndirectedWeightMatrix();
};
