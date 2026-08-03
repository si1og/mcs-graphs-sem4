#!/usr/bin/env bash
set -e

BUILD_DIR="build"
BUILD_TYPE="${BUILD_TYPE:-Release}"
LAB="${1:-lab2}"

cmake -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
cmake --build "$BUILD_DIR" -j

if [ "${1:-}" = "test" ]; then
    ctest --test-dir "$BUILD_DIR" --output-on-failure
    exit 0
fi

./"$BUILD_DIR"/cli/cli
