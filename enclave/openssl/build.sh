make clean
# configure native openssl
#CC=x86_64-linux-musl-gcc ./Configure --prefix=/usr/local/ssl --openssldir=/usr/local/ssl linux-x86_64 no-shared no-async no-engine no-dso no-asm -g3 -O0 -fno-omit-frame-pointer -fno-inline-functions -fvisibility=hidden -fpie -ffunction-sections -fdata-sections -fstack-protector -DOPENSSL_NO_SECURE_MEMORY -fPIC -D_FORTIFY_SOURCE=0 -O0 --debug
# no-asm: required to remove SIGILL, Illegal instruction
#CC=x86_64-linux-musl-gcc ./config  -D_FORTIFY_SOURCE=0 -fPIC no-engines no-asm no-dso no-hw no-shared no-threads no-async -d
CC=gcc ./config  -D_FORTIFY_SOURCE=0 -fPIC no-asm no-dso no-hw no-shared no-threads no-async
make depend
make build_apps -j8
make build_engines -j8
make build_apps -j8
#
#  Build native openssl
#$ CC=musl-gcc make build_apps -j
#
#  Install openssl
#  $ sudo make install
#
cp libcrypto.a ../lib
cp libssl.a ../lib
