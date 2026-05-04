#!/usr/bin/env bash
set -e

BUILD_DIR="build"
BUILD_TYPE="${BUILD_TYPE:-Release}"
LAB="${1:-lab2}"

if [ ! -d "$BUILD_DIR" ]; then
    cmake -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
fi

cmake --build "$BUILD_DIR" -j

./"$BUILD_DIR"/cli/"$LAB"
