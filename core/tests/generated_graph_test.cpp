#include "test_utils.h"

void testGeneratedGraph() {
    GeneratorGraph graph(11);
    graph.generate();

    const Matrix& adjacency = graph.getUndirectedAdjacencyMatrix();
    requireSymmetric(adjacency);
    require(isConnected(adjacency), "generated graph must be connected");

    for (int i = 0; i < adjacency.rows(); ++i) {
        require(adjacency(i, i) == 0, "generated graph must not have loops");
    }
}

int main() {
    return runTest("generated graph", testGeneratedGraph);
}
