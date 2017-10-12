#!/bin/bash

set -euxo pipefail

g++ -coverage -std=c++11 -Wall -Werror -Weffc++ gpgraph_test.cc -lgmock_main -lpthread
