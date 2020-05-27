#!/bin/sh
version=6.1
tarname=ncurses-$version.tar.gz
url=https://ftp.gnu.org/pub/gnu/ncurses/ncurses-$version.tar.gz

if [ ! -f $tarname ]
then
	wget $url
fi
tar -xvf $tarname
cd ncurses-$version

rm -rf build;
mkdir build;
cd build
../configure CFLAGS="-D_FORTIFY_SOURCE=0 -fPIC" --prefix=$PWD/../install
make
cp lib/*.a ../../../lib
