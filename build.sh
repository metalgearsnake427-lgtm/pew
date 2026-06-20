#!/bin/bash

set -e

echo "Building VIDM..."

mkdir -p build
cd build

cmake ..
make

echo "VIDM build complete!"