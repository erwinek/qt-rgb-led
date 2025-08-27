#!/bin/bash
set -e

PROJECT_DIR=~/qt-rgb-led
BUILD_DIR=$PROJECT_DIR/build

mkdir -p $BUILD_DIR
cd $BUILD_DIR

cmake ..
make -j$(nproc)

echo "✅ Build finished! Binary: $BUILD_DIR/qt-rgb-led"
