#!/bin/bash

set -euxo pipefail

cd build
ninja test
