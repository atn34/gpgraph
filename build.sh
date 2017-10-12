#!/bin/bash

set -euxo pipefail

mkdir -p build
if ! [ -f build/ninja ] ; then
    wget https://github.com/ninja-build/ninja/releases/download/v1.8.2/ninja-linux.zip
    unzip ninja-linux.zip
    mv ninja build
    rm ninja-linux.zip
fi
meson -Db_coverage=true build
cd build
./ninja
