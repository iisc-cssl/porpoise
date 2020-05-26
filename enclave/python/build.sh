rm -rf build
mkdir build
cd build
CC="gcc" CFLAGS="-fPIC -D_FORTIFY_SOURCE=0" ../configure --prefix=$PWD/../install
make python sharedmods -j8
make install
rm Programs/_testembed.o
#make -j8


