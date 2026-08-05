#pragma once

#include "matrix.h"

#include <vector>

struct WeightedNeighbor {
    int vertex = 0;
    double weight = 0;
};

using AdjacencyList = std::vector<std::vector<int>>;
using WeightedAdjacencyList = std::vector<std::vector<WeightedNeighbor>>;

class Graph {
public:
    explicit Graph(int vertexCount);
    virtual ~Graph() = default;

    int getVertexCount() const;
    const Matrix& getAdjacencyMatrix() const;
    const Matrix& getWeightMatrix() const;
    const Matrix& getUndirectedAdjacencyMatrix() const;
    const Matrix& getUndirectedWeightMatrix() const;
    AdjacencyList getAdjacencyList() const;
    WeightedAdjacencyList getWeightedAdjacencyList() const;

    void printAdjacencyMatrix() const;
    void printWeightMatrix() const;
    void printUndirectedAdjacencyMatrix() const;
    void printUndirectedWeightMatrix() const;

    struct isMatrixInit {
        bool adjacency = false;
        bool weight = false;
        bool capacity = false;
        bool cost = false;
        bool eulerian = false;
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

    template<typename Item, typename ItemFactory>
    std::vector<std::vector<Item>> m_matrixToAdjacencyList(
        const Matrix& adjacency,
        ItemFactory createItem
    ) const {
        std::vector<std::vector<Item>> result(adjacency.rows());

        for (int from = 0; from < adjacency.rows(); ++from) {
            for (int to = 0; to < adjacency.cols(); ++to) {
                if (from != to && adjacency(from, to) != 0) {
                    result[from].push_back(createItem(from, to));
                }
            }
        }

        return result;
    }
};
