#include "test_utils.h"

void testEulerianTransformation() {
    TestGeneratorGraph path(4);
    path.setUndirectedEdges({{0, 1}, {1, 2}, {2, 3}});

    const auto pathResult = path.checkIfEulerianGraph();
    require(pathResult.transformationCompleted, "path graph must be transformed");
    require(
        pathResult.resultIsConnected,
        "safe transformation must preserve connectivity"
    );
    requireEvenDegrees(pathResult.eulerianAdjacencyMatrix);
    require(
        isConnected(pathResult.eulerianAdjacencyMatrix),
        "Eulerian result must be connected"
    );
    require(
        !pathResult.eulerianCycle.empty(),
        "Eulerian cycle must be constructed"
    );
    require(
        pathResult.eulerianCycle.front() == pathResult.eulerianCycle.back(),
        "Eulerian cycle must finish at its start vertex"
    );

    Matrix remainingEdges = pathResult.eulerianAdjacencyMatrix;

    for (size_t i = 1; i < pathResult.eulerianCycle.size(); ++i) {
        const int from = pathResult.eulerianCycle[i - 1];
        const int to = pathResult.eulerianCycle[i];

        require(remainingEdges(from, to) != 0, "cycle uses a missing edge");
        remainingEdges(from, to) = 0;
        remainingEdges(to, from) = 0;
    }

    for (int i = 0; i < remainingEdges.rows(); ++i) {
        for (int j = 0; j < remainingEdges.cols(); ++j) {
            require(remainingEdges(i, j) == 0, "cycle must use every edge");
        }
    }

    TestGeneratorGraph singleEdge(2);
    singleEdge.setUndirectedEdges({{0, 1}});

    const auto safeResult = singleEdge.checkIfEulerianGraph();
    require(
        !safeResult.transformationCompleted,
        "bridge removal must require confirmation"
    );

    const auto disconnectedResult = singleEdge.checkIfEulerianGraph(true);
    require(
        disconnectedResult.transformationCompleted,
        "allowed bridge removal must complete"
    );
    require(
        !disconnectedResult.resultIsConnected,
        "bridge removal must mark the result disconnected"
    );
    requireEvenDegrees(disconnectedResult.eulerianAdjacencyMatrix);
    require(
        disconnectedResult.eulerianCycle.empty(),
        "disconnected result must not have a common Eulerian cycle"
    );
}

int main() {
    return runTest("Eulerian transformation", testEulerianTransformation);
}
