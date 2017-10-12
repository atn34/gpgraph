#!/bin/bash

set -euxo pipefail

mkdir -p build
meson -Db_coverage=true build
cd build
ninja
