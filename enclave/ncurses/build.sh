#!/bin/sh
rm -rf build;
mkdir build;
cd build
../configure CFLAGS="-D_FORTIFY_SOURCE=0 -O0 -g -fPIC" --prefix=$PWD/_inst
make
cp lib/*.a ../../lib
