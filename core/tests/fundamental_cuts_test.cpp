#include "test_utils.h"

void testFundamentalCuts() {
    TestGeneratorGraph graph(4);
    graph.setUndirectedWeightedEdges({
        {0, 1, 1},
        {1, 2, 2},
        {2, 3, 3},
        {0, 2, 10},
        {1, 3, 10}
    });

    const auto result = graph.buildFundamentalCutSystem();
    require(result.success, "fundamental cut system must be built");
    require(result.spanningTree.success, "minimum spanning tree must be returned");
    require(result.spanningTree.edges.size() == 3, "tree must have n - 1 edges");
    require(
        std::abs(result.spanningTree.totalWeight - 6) < TEST_EPS,
        "wrong minimum spanning tree weight"
    );
    require(result.fundamentalCuts.size() == 3, "tree must produce n - 1 cuts");

    require(
        result.fundamentalCuts[0].cut == GraphCut({{0, 1}, {0, 2}}),
        "wrong cut for tree edge 0-1"
    );
    require(
        result.fundamentalCuts[1].cut == GraphCut({{0, 2}, {1, 2}, {1, 3}}),
        "wrong cut for tree edge 1-2"
    );
    require(
        result.fundamentalCuts[2].cut == GraphCut({{1, 3}, {2, 3}}),
        "wrong cut for tree edge 2-3"
    );

    const GraphCut symmetricDifference =
        graph.symmetricDifferenceOfFundamentalCuts(result, {0, 1});
    require(
        symmetricDifference == GraphCut({{0, 1}, {1, 2}, {1, 3}}),
        "symmetric difference must cancel common edges"
    );

    const GraphCut zero =
        graph.symmetricDifferenceOfFundamentalCuts(result, {2, 2});
    require(zero.empty(), "cut XOR itself must be empty");
}

int main() {
    return runTest("Fundamental cuts", testFundamentalCuts);
}
