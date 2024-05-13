#!/bin/bash

ROOT_PATH=$(pwd)
CUSTOM_KS_DIR="$ROOT_PATH/ModifKSharkBuild"
CUSTOM_KS_LIBDIR="$ROOT_PATH/Stacklook/lib"
BUILD_OPTIONS="-D_DOXYGEN_DOC=1 -D_INSTALL_PREFIX=$CUSTOM_KS_DIR -D_LIBDIR=$CUSTOM_KS_LIBDIR -DCMAKE_BUILD_TYPE=Debug"

mkdir ModifKSharkBuild
mkdir Stacklook/lib
cd KS_fork/build  # Go into build folder
cmake ${BUILD_OPTIONS} ../ # Build with appropriate options
make
sudo ./install_gui.sh
sudo ./install_libkshark-devel.sh
cd ../../ # Go back to original directory
