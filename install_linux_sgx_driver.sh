#!/bin/sh
#version=2.5
#sudo ./install_driver.sh
#url=https://github.com/intel/linux-sgx-driver/archive/sgx_driver_2.5.tar.gz
#tarname=sgx_driver_$version.tar.gz
#if [ ! -f $tarname ]
#then
#	wget $url;
#fi
#
#mkdir -p linux-sgx-driver
#tar -xvf $tarname -C linux-sgx-driver --strip 1
git clone https://github.com/intel/linux-sgx-driver.git
cd linux-sgx-driver
git checkout sgx_driver_2.5

sudo apt-get install -y linux-headers-$(uname -r)

make

sudo mkdir -p "/lib/modules/"`uname -r`"/kernel/drivers/intel/sgx"    
sudo cp isgx.ko "/lib/modules/"`uname -r`"/kernel/drivers/intel/sgx"    
sudo sh -c "cat /etc/modules | grep -Fxq isgx || echo isgx >> /etc/modules"    
sudo /sbin/depmod
sudo /sbin/modprobe isgx
