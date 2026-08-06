#pragma once

#include "generator_graph.h"

#include <cmath>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

inline constexpr double TEST_EPS = 1e-9;

inline void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

inline int runTest(const std::string& name, void (*test)()) {
    try {
        test();
        std::cout << "[PASS] " << name << "\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "[FAIL] " << name << ": " << error.what() << "\n";
        return 1;
    }
}

inline bool isConnected(const Matrix& adjacency) {
    if (adjacency.rows() == 0) {
        return true;
    }

    std::vector<bool> visited(adjacency.rows(), false);
    std::vector<int> stack = {0};
    visited[0] = true;

    while (!stack.empty()) {
        const int vertex = stack.back();
        stack.pop_back();

        for (int to = 0; to < adjacency.cols(); ++to) {
            if (adjacency(vertex, to) != 0 && !visited[to]) {
                visited[to] = true;
                stack.push_back(to);
            }
        }
    }

    for (bool value : visited) {
        if (!value) {
            return false;
        }
    }

    return true;
}

inline void requireSymmetric(const Matrix& matrix) {
    require(matrix.rows() == matrix.cols(), "matrix must be square");

    for (int i = 0; i < matrix.rows(); ++i) {
        for (int j = 0; j < matrix.cols(); ++j) {
            require(
                std::abs(matrix(i, j) - matrix(j, i)) < TEST_EPS,
                "matrix must be symmetric"
            );
        }
    }
}

inline void requireEvenDegrees(const Matrix& adjacency) {
    for (int i = 0; i < adjacency.rows(); ++i) {
        int degree = 0;

        for (int j = 0; j < adjacency.cols(); ++j) {
            if (adjacency(i, j) != 0) {
                ++degree;
            }
        }

        require(degree % 2 == 0, "all vertex degrees must be even");
    }
}

class TestGeneratorGraph : public GeneratorGraph {
public:
    explicit TestGeneratorGraph(int vertexCount)
        : GeneratorGraph(vertexCount) {}

    void setUndirectedEdges(const std::vector<Edge>& edges) {
        m_undirectedAdjacencyMatrix = Matrix(m_vertexCount, m_vertexCount, 0);

        for (const auto& edge : edges) {
            m_undirectedAdjacencyMatrix(edge.from, edge.to) = 1;
            m_undirectedAdjacencyMatrix(edge.to, edge.from) = 1;
        }

        isMatrixInit.adjacency = true;
    }

    void setUndirectedWeightedEdges(const std::vector<WeightedEdge>& edges) {
        m_undirectedAdjacencyMatrix = Matrix(m_vertexCount, m_vertexCount, 0);
        m_undirectedWeightMatrix = Matrix(
            m_vertexCount,
            m_vertexCount,
            std::numeric_limits<double>::infinity()
        );

        for (int vertex = 0; vertex < m_vertexCount; ++vertex) {
            m_undirectedWeightMatrix(vertex, vertex) = 0;
        }

        for (const auto& edge : edges) {
            m_undirectedAdjacencyMatrix(edge.from, edge.to) = 1;
            m_undirectedAdjacencyMatrix(edge.to, edge.from) = 1;
            m_undirectedWeightMatrix(edge.from, edge.to) = edge.weight;
            m_undirectedWeightMatrix(edge.to, edge.from) = edge.weight;
        }

        isMatrixInit.adjacency = true;
        isMatrixInit.weight = true;
    }
};
