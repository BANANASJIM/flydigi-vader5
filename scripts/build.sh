#!/bin/bash
set -euo pipefail

BUILD_TYPE="${1:-Release}"
BUILD_DIR="build"

cmake -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
cmake --build "$BUILD_DIR" -j"$(nproc)"
