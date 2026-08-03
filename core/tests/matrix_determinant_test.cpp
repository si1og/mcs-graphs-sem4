#include "test_utils.h"

void testDeterminant() {
    Matrix matrix(3, 3, 0);
    matrix(0, 0) = 1;
    matrix(0, 1) = 2;
    matrix(0, 2) = 3;
    matrix(1, 1) = 4;
    matrix(1, 2) = 5;
    matrix(2, 0) = 1;
    matrix(2, 2) = 6;

    require(
        std::abs(matrix.determinant() - 22) < TEST_EPS,
        "wrong determinant"
    );
}

int main() {
    return runTest("matrix determinant", testDeterminant);
}
