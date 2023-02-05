#!/usr/bin/env bash

rm -rf build
mkdir build
	cmake -DQT_STATIC_DIR=$(QT_STATIC_DIR) -DCMAKE_BUILD_TYPE=Release \
		-S . -B build
cmake -B build -S .
cmake --build build --parallel
