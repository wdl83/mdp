#! /bin/bash

set -euo pipefail

if [[ "" != "${1-}" ]]
then
    BUILD_DIR="$1"
else
    echo "BUILD_DIR missing"
fi

cmake --build ${BUILD_DIR} --target run_all_tests
