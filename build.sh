#!/bin/sh
mkdir -p enclave/lib
(cd enclave/musl;sh build.sh)
(cd enclave/openssl; sh build.sh)
(cd enclave/readline; sh build.sh)
(cd enclave/ncurses; sh build.sh)
(cd enclave/zlib; sh build.sh)
(cd enclave/bzip2; sh build.sh)
(cd enclave/libevent; sh build.sh)
echo $PWD
