sh autogen.sh
CC="gcc -D_FORTIFY_SOURCE=0" ./configure --prefix=$PWD/_install CFLAGS="-fPIC -D_FORTIFY_SOURCE=0"
make install-am
cp _install/lib/*.a ../lib
