./autogen.sh
./configure --with-libevent=$PWD/../libevent/install CFLAGS="-fPIC"
make memcached
