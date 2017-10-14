#!/bin/bash

set -euxo pipefail

${CXX:-g++} -coverage -std=c++11 -Wall -Wextra -Werror -Wshadow -Wnon-virtual-dtor -pedantic -Weffc++ gpgraph_test.cc -lgmock_main -lpthread
