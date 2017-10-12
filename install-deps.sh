#! /bin/bash

set -euxo pipefail

# googletest

GOOGLETEST_VERSION=1.8.0
GOOGLETEST_TARBALL=googletest-${GOOGLETEST_VERSION}.tar.gz
wget https://github.com/google/googletest/archive/release-${GOOGLETEST_VERSION}.tar.gz -O ${GOOGLETEST_TARBALL}
tar -xvf ${GOOGLETEST_TARBALL}
rm ${GOOGLETEST_TARBALL}
pushd googletest-release-${GOOGLETEST_VERSION}
mkdir -p build
pushd build
cmake ..
make -j8
sudo make install
popd
popd
rm -rf googletest-release-${GOOGLETEST_VERSION}

