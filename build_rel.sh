#! /bin/bash

set -euo pipefail

if [[ "" != "${1-}" ]]
then
    BUILD_DIR="$1"
else
    echo "BUILD_DIR missing"
fi

if [[ "" != "${2-}" ]]
then
    INSTALL_DIR="$2"
else
    echo "INSTALL_DIR missing"
    exit 1
fi

cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" \
    -S . -B ${BUILD_DIR}
cmake --build ${BUILD_DIR}
