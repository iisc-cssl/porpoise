#!/bin/sh
version=8.0
url=https://ftp.gnu.org/gnu/readline/readline-8.0.tar.gz
tarname=readline-$version.tar.gz
if [ ! -f $tarname ]
then
	wget $url
fi
tar -xvf $tarname

cd readline-$version
rm -rf build
mkdir build
cd build
#../configure CC=gcc CFLAGS="-fPIC -D_FORTIFY_SOURCE=0" --no-create --no-recursion
../configure CC=gcc CFLAGS="-fPIC -D_FORTIFY_SOURCE=0"
make
echo $PWD
cp libreadline.a ../../../lib
cp libhistory.a ../../../lib
