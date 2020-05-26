#./configure CFLAGS="-fPIC -O0 -g -ffunction-sections -fdata-sections" CC="x86_64-linux-musl-gcc" --prefix=$PWD/install --syslibdir=/lib --enable-warning --disable-shared --enable-debug --disable-optimize --with-pic
rm -rf build
mkdir build
cd build
../configure CFLAGS="-fPIC -O0 -g -ffunction-sections -fdata-sections -D_FORTIFY_SOURCE=0" --prefix=$PWD/../install --syslibdir=/lib --enable-warning --disable-shared --enable-debug --disable-optimize --with-pic
make -j32  
make install
cp lib/*.a ../../lib
