#include "test_utils.h"

void testFlows() {
    GeneratorGraph graph(9);
    graph.generate();
    graph.generateCapacityAndCostMatrices();

    const int source = 0;
    const int sink = 8;
    const auto maxFlow = graph.fordFulkerson(source, sink);
    const Matrix& capacity = graph.getCapacityMatrix();

    require(maxFlow.maxFlow > 0, "generated network must have a positive flow");

    for (int i = 0; i < capacity.rows(); ++i) {
        for (int j = 0; j < capacity.cols(); ++j) {
            require(maxFlow.flowMatrix(i, j) >= -TEST_EPS, "flow must be non-negative");
            require(
                maxFlow.flowMatrix(i, j) <= capacity(i, j) + TEST_EPS,
                "flow must not exceed capacity"
            );
        }
    }

    for (int vertex = 1; vertex < sink; ++vertex) {
        double incoming = 0;
        double outgoing = 0;

        for (int other = 0; other < capacity.rows(); ++other) {
            incoming += maxFlow.flowMatrix(other, vertex);
            outgoing += maxFlow.flowMatrix(vertex, other);
        }

        require(
            std::abs(incoming - outgoing) < TEST_EPS,
            "flow conservation is violated"
        );
    }

    const int requiredFlow = std::max(1, maxFlow.maxFlow / 2);
    const auto minCost = graph.minCostFlow(source, sink, requiredFlow);
    require(minCost.success, "minimum-cost flow must reach the requested value");
    require(minCost.achievedFlow == requiredFlow, "wrong minimum-cost flow value");

    double matrixCost = 0;
    for (int i = 0; i < minCost.costFlowMatrix.rows(); ++i) {
        for (int j = 0; j < minCost.costFlowMatrix.cols(); ++j) {
            matrixCost += minCost.costFlowMatrix(i, j);
        }
    }

    require(
        std::abs(matrixCost - minCost.totalCost) < TEST_EPS,
        "wrong minimum flow cost matrix"
    );
}

int main() {
    return runTest("flows", testFlows);
}
