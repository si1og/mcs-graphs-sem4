#pragma once

/*
 * SCALE -> a > 0
 * SHAPE -> c > 0
 * SHIFT -> y0
 */

#pragma once

struct WeibullParams {
    double scale;
    double shape;
    double shift;
};

namespace constants {

inline constexpr WeibullParams graph {
    .scale = 0.15,
    .shape = 2.0,
    .shift = 0.05
};

inline constexpr WeibullParams weight {
    .scale = 0.3,
    .shape = 2.0,
    .shift = 0.1
};

};
