#!/bin/bash
set -e

cmake -B release -S . -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
cmake --build release

sudo systemctl daemon-reload
