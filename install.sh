#!/bin/bash
set -e

cmake -B release -S . \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/usr

cmake --build release --parallel

sudo cmake --install release

sudo systemctl daemon-reload
