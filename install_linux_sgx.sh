#!/bin/sh
#version=2.4
#url=https://github.com/intel/linux-sgx/archive/sgx_$version.tar.gz
#tarname=sgx_$version.tar.gz
#if [ ! -f $tarname ]
#then
#	wget $url
#fi
#mkdir -p linux-sgx
#tar -xvf $tarname -C linux-sgx --strip 1
git clone https://github.com/intel/linux-sgx.git
cd linux-sgx
git checkout sgx_2.4
./download_prebuilt.sh

sudo apt-get install build-essential ocaml automake autoconf libtool wget python libssl-dev -y
sudo apt-get install libssl-dev libcurl4-openssl-dev protobuf-compiler libprotobuf-dev debhelper cmake -y
sudo apt-get install libssl-dev libcurl4-openssl-dev libprotobuf-dev -y
sudo apt-get install build-essential python -y

make sdk
make sdk_install_pkg
printf "no\n/opt/intel/porpoise\n\n"| sudo ./linux/installer/bin/sgx_linux_x64_sdk_*.bin
make psw
make psw_install_pkg
sudo ./linux/installer/bin/sgx_linux_x64_psw_*.bin
cd ..


