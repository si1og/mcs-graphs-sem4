#pragma once

#include "matrix.h"

struct MaxFlowResult {
    Matrix flowMatrix;

    int maxFlow = 0;
    int iterations = 0;

    MaxFlowResult(int vertexCount)
        : flowMatrix(vertexCount, vertexCount, 0)
    {}
};

struct MinCostFlowResult {
    Matrix flowMatrix;
    int requiredFlow = 0;
    int achievedFlow = 0;

    long long totalCost = 0;

    int iterations = 0;

    bool success = false;
};
