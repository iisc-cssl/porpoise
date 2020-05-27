version=8.43
tarname=pcre-$version.tar.gz
url=https://ftp.pcre.org/pub/pcre/$tarname

if [ ! -f $tarname ]
then
	wget $url
fi


tar -xvf $tarname

cd pcre-$version

rm -rf build
mkdir build
cd build
CFLAGS="-fPIC" ../configure 
make libpcre.la
cp .libs/*.a ../../../lib
cd ../../..
