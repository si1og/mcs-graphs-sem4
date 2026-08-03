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
}

int main() {
    return runTest("Eulerian transformation", testEulerianTransformation);
}
