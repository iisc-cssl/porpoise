#!/bin/sh
rm -rf build
mkdir build
cd build
#../configure CC=gcc CFLAGS="-fPIC -D_FORTIFY_SOURCE=0" --no-create --no-recursion
../configure CC=gcc CFLAGS="-fPIC -D_FORTIFY_SOURCE=0"
make
echo $PWD
cp libreadline.a ../../lib
cp libhistory.a ../../lib
