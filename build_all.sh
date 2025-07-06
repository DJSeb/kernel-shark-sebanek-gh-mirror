#!/bin/bash

BUILD_OPTS='-D_DOXYGEN_DOC=1 -DCMAKE_BUILD_TYPE=Release'
CURR_DIR="$(pwd)"

printf "%s\n" ${CURR_DIR}

# Build modified KernelShark
mkdir -p KS_fork/build
cd KS_fork/build
cmake ${BUILD_OPTS} .. # Build with appropriate opts
make
sudo ./install_gui.sh
sudo ./install_libkshark-devel.sh
cd ${CURR_DIR} # Get back here

# Build Stacklook for modified KernelShark
mkdir -p Stacklook/build
cd Stacklook/build
cmake ..
make
cd ${CURR_DIR}

# Build Stacklook for unmodified KernelShark
mkdir -p Stacklook/unmodif_build
cd Stacklook/unmodif_build
cmake -D_UNMODIFIED_KSHARK=1 ..
make
cd ${CURR_DIR}

# Build Naps for modified KernelShark
mkdir -p Naps/build
cd Naps/build
cmake ..
make
cd ${CURR_DIR}

# Build Naps for unmodified KernelShark
mkdir -p Naps/unmodif_build
cd Naps/unmodif_build
cmake -D_UNMODIFIED_KSHARK=1 ..
make
cd ${CURR_DIR}

# Build NoBoxes, only for modified KernelShark
mkdir -p NoBoxes/build
cd NoBoxes/build
cmake ..
make
cd ${CURR_DIR}

