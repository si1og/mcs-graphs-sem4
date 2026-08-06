#include "test_utils.h"

void testKruskalAndPrufer() {
    GeneratorGraph graph(10);
    graph.generate();
    graph.generateWeightMatrix(WeightMode::Positive);

    const auto result = graph.kruskalMinimumSpanningTree();
    require(result.kruskal.success, "Kruskal must build a spanning tree");
    require(result.kruskal.edges.size() == 9, "spanning tree must have n - 1 edges");
    require(
        result.pruferCode.size() == 9,
        "weighted Prufer code must store n - 1 edges"
    );
    require(
        result.pruferRoundTripSuccess,
        "decoded Prufer tree must match the source tree"
    );

    double edgeWeightSum = 0;
    for (const auto& edge : result.kruskal.edges) {
        edgeWeightSum += edge.weight;
    }

    require(
        std::abs(edgeWeightSum - result.kruskal.totalWeight) < TEST_EPS,
        "wrong spanning tree weight"
    );
}

int main() {
    return runTest("Kruskal and Prufer", testKruskalAndPrufer);
}
