#include "test_utils.h"

void testGeneratedGraph() {
    GeneratorGraph graph(11);
    graph.generate();

    const Matrix& adjacency = graph.getUndirectedAdjacencyMatrix();
    requireSymmetric(adjacency);
    require(isConnected(adjacency), "generated graph must be connected");

    const auto adjacencyList = graph.getAdjacencyList();

    for (int from = 0; from < graph.getVertexCount(); ++from) {
        for (int to = 0; to < graph.getVertexCount(); ++to) {
            const bool found = std::find(
                adjacencyList[from].begin(),
                adjacencyList[from].end(),
                to
            ) != adjacencyList[from].end();

            require(
                found == (graph.getAdjacencyMatrix()(from, to) != 0),
                "adjacency list must match the adjacency matrix"
            );
        }
    }

    graph.generateWeightMatrix(WeightMode::Positive);
    const auto weightedList = graph.getWeightedAdjacencyList();

    for (int from = 0; from < graph.getVertexCount(); ++from) {
        for (const auto& neighbor : weightedList[from]) {
            require(
                std::abs(
                    neighbor.weight
                    - graph.getWeightMatrix()(from, neighbor.vertex)
                ) < TEST_EPS,
                "weighted adjacency list must preserve edge weights"
            );
        }
    }

    for (int i = 0; i < adjacency.rows(); ++i) {
        require(adjacency(i, i) == 0, "generated graph must not have loops");
    }
}

int main() {
    return runTest("generated graph", testGeneratedGraph);
}
