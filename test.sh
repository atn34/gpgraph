#!/bin/bash

set -euxo pipefail

./a.out

gcov gpgraph_test.cc > /dev/null
