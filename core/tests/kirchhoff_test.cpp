#include "test_utils.h"

void testKirchhoff() {
    GeneratorGraph graph(8);
    graph.generate();

    const auto result = graph.countSpanningTreesKirchhoff();
    require(result.count > 0, "connected graph must have a spanning tree");
    require(
        std::abs(result.cofactorDeterminant - result.count) < TEST_EPS,
        "Kirchhoff determinant must equal the spanning tree count"
    );

    for (int i = 0; i < result.kirchhoffMatrix.rows(); ++i) {
        double rowSum = 0;

        for (int j = 0; j < result.kirchhoffMatrix.cols(); ++j) {
            rowSum += result.kirchhoffMatrix(i, j);
        }

        require(
            std::abs(rowSum) < TEST_EPS,
            "Kirchhoff row sum must be zero"
        );
    }
}

int main() {
    return runTest("Kirchhoff", testKirchhoff);
}
