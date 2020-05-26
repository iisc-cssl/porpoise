(rm -rf build; mkdir build;cd build; OPENSSL_ROOT_DIR=$PWD/../../openssl cmake .. -DWITH_MRUBY=off; make h2o; cp -r ../examples .)
