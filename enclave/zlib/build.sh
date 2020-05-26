#!/bin/sh

rm -rf build;
mkdir build;
cd build;
CFLAGS="-fPIC -DFORTIFY_SOURCE=0" ../configure 
make
cp libz.a ../../lib

