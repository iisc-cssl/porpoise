if [ ! -f libevent-2.1.11-stable.tar.gz ]
then
	wget https://github.com/libevent/libevent/releases/download/release-2.1.11-stable/libevent-2.1.11-stable.tar.gz
fi
tar -xvf libevent-2.1.11-stable.tar.gz
cd libevent-2.1.11-stable
sh autogen.sh
rm -rf build
mkdir build
cd build
../configure --prefix=$PWD/../../install CFLAGS="-fPIC -D_FORTIFY_SOURCE=0"
make
make install-am
cp ../../install/lib/*.a ../../../lib

