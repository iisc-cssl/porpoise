rm -rf build
mkdir build
cd build
CFLAGS="-fPIC" ../configure 
make libpcre.la
