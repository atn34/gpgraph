#!/bin/bash

set -euxo pipefail

g++ -std=c++11 -Weffc++ -Werror gpgraph.h

mkdir -p build
if ! [ -f build/ninja ] ; then
    wget https://github.com/ninja-build/ninja/releases/download/v1.8.2/ninja-linux.zip
    unzip ninja-linux.zip
    mv ninja build
    rm ninja-linux.zip
fi
PATH="$PATH:./build" meson -Db_coverage=true build
cd build
./ninja
