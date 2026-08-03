#include "test_utils.h"

#include <functional>

int bruteForceMaximumMatching(const Matrix& adjacency) {
    const int vertexCount = adjacency.rows();
    std::vector<bool> used(vertexCount, false);

    std::function<int()> search = [&]() {
        int first = -1;

        for (int i = 0; i < vertexCount; ++i) {
            if (!used[i]) {
                first = i;
                break;
            }
        }

        if (first == -1) {
            return 0;
        }

        used[first] = true;
        int best = search();

        for (int to = first + 1; to < vertexCount; ++to) {
            if (!used[to] && adjacency(first, to) != 0) {
                used[to] = true;
                best = std::max(best, 1 + search());
                used[to] = false;
            }
        }

        used[first] = false;
        return best;
    };

    return search();
}

void testEdmondsBlossom() {
    GeneratorGraph graph(8);
    graph.generate();

    const auto result = graph.edmondsBlossomInOriginalGraph();
    require(result.success, "Edmonds Blossom must complete");
    require(
        static_cast<int>(result.edges.size())
            == bruteForceMaximumMatching(graph.getUndirectedAdjacencyMatrix()),
        "Edmonds Blossom result must be maximum"
    );

    std::vector<bool> used(8, false);
    for (const auto& edge : result.edges) {
        require(
            !used[edge.from] && !used[edge.to],
            "matching edges must be independent"
        );
        used[edge.from] = true;
        used[edge.to] = true;
    }
}

int main() {
    return runTest("Edmonds Blossom", testEdmondsBlossom);
}
