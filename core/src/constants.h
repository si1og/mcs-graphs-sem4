#pragma once

/*
 * SCALE -> a > 0
 * SHAPE -> c > 0
 * SHIFT -> y0
 */

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

inline constexpr WeibullParams capacity {
    .scale = 0.5,
    .shape = 2.0,
    .shift = 0.2
};

inline constexpr WeibullParams cost {
    .scale = 0.4,
    .shape = 2.0,
    .shift = 0.1
};

}
