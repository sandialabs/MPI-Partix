#!/bin/bash

# Provides an example of how to configure Partix with cmake
# ---------------------------------------------------------

cd build
cmake ..  -DCMAKE_CXX_COMPILER=mpicxx -DCMAKE_LINKER=mpicxx -DQthreads_ROOT=$QTHREADS_INSTALL_PATH -DPartix_ENABLE_QTHREADS=ON
