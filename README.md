<img src="porpoise-logo.png" height=200>

## Porpoise: A tool to port commodity application to Intel SGX

### Requirements

* Intel 6th generation skylake CPU or later which provide support for intel SGX.

* Ubuntu 16.04 

* Linux 4.4.0-169

* 16 GB Ram(Recommended)


### Install Linux SGX Driver
Install linux SGX driver 2.5
[https://github.com/intel/linux-sgx-driver/tree/sgx_driver_2.5](https://github.com/intel/linux-sgx-driver/tree/sgx_driver_2.5)

Execute the script `install_linux_sgx_driver.sh` which clone the github repository of linux sgx driver into `linux-sgx-driver` and checkout to version `sgx_driver_2.5` and build and install the driver.
```
sh install_linux_sgx_driver.sh
```
If the above script fails, execute the following commands:-

```
git clone https://github.com/intel/linux-sgx-driver.git
git checkout sgx_driver_2.5
sudo apt-get install linux-headers-$(uname -r)
make
sudo mkdir -p "/lib/modules/"`uname -r`"/kernel/drivers/intel/sgx"
sudo cp isgx.ko "/lib/modules/"`uname -r`"/kernel/drivers/intel/sgx"
sudo sh -c "cat /etc/modules | grep -Fxq isgx || echo isgx >> /etc/modules"
sudo /sbin/depmod
sudo /sbin/modprobe isgx
```

### Install Linux SGX SDK
Install Linux 2.4 Open Source Gold Release
[https://github.com/intel/linux-sgx/tree/sgx_2.4](https://github.com/intel/linux-sgx/tree/sgx_2.4)

Run the following script for installing Linux SGX SDK. It will ask for super user password to install dependencies.
```
sh install_linux_sgx.sh
```

If the script fails execute the following commands:-
```
wget https://github.com/intel/linux-sgx/archive/sgx_2.4.tar.gz
mkdir -p linux-sgx
tar -xvf sgx_2.4.tar.gz -C linux-sgx --strip 1

cd linux-sgx
./download_prebuilt.sh

sudo apt-get install build-essential ocaml automake autoconf libtool wget python libssl-dev -y
sudo apt-get install libssl-dev libcurl4-openssl-dev protobuf-compiler libprotobuf-dev debhelper cmake -y
sudo apt-get install libssl-dev libcurl4-openssl-dev libprotobuf-dev -y
sudo apt-get install build-essential python -y
sudo apt-get install libnss-mdns -y

make sdk
make sdk_install_pkg
./linux/installer/bin/sgx_linux_x64_sdk_*.bin #it will ask for location, accept default by typing "yes" and hit enter when ask for directory name.
make psw
make psw_install_pkg
./linux/installer/bin/sgx_linux_x64_psw_*.bin

```

### Build Porpoise and Benchmark Applications


```
git clone https://github.com/iisc-cssl/porpoise.git
cd porpoise
sh build.sh
make h2o memcached python openssl
./openssl version
./python --version
./memcached --version
./h2o --version
```

### Porting new application to Intel SGX with Porpoise.

There are three main steps in porting applications to SGX with Porpoise.

1. Compile the application with position independent code.
2. Compile the dependend libraries with position independend/independent code.
3. Link application with Porpoise.

### Porting libjpeg to Porpoise
In this tutorial we will port libjpeg to Intel SGX using Porpoise

1. Obtain the source code of libpeg into `porpoise/enclave/libjpeg` folder.
```
cd porpoise/enclave
mkdir libjpeg
cd libjpeg
wget http://www.ijg.org/files/jpegsrc.v6b.tar.gz
tar -xvf jpegsrc.v6b.tar.gz --strip 1
```
2. Compile libjpeg
```
mkdir build
cd build
../configure CFLAGS="-fPIC"
make cjpeg
./cjpeg -h
```
3. Compiling dependencies of libjpeg
```
ldd cjpeg

>	linux-vdso.so.1 =>  (0x00007ffc131e5000)
>       libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007fb801dbc000)
>       /lib64/ld-linux-x86-64.so.2 (0x00007fb802186000)
```
cjpeg depends on three shared libraries. Ignore `linux-vdso.so.1` and `ld-linux-x86-64.so.2`. For `libc.so.6`, Porpoise uses musl as libc which is already present in Porpoise at `porpoise/enclave/musl`. For building musl refer to `propoise/enclave/musl/build.sh`.

4. compile Porpoise and link it with cjpeg
	
Create new file Makefile.cjpeg
```
cp Makefile.sample Makefile.cjpeg
```
Update `Makefile.cjpeg` as following
```
application_name := cjpeg
native_application_location := enclave/libjpeg/build
```
 
compile and link Porpoise with cjpeg
```
make -f Makefile.cjpg
./cjpeg -greyscale -dct int -progressive -opt -outfile testoutp.jpg enclave/libjpeg/testimg.ppm
```
### Here some of the common errors which developers come across when porting an application to Intel SGX with Porpoise.

* undefined reference
```
convert.c:(.text+0x193f): undefined reference to `xstrdup'
```
_solution_: Add the missing library that provide the defination of given symbol.

* multiple reference 
```
enclave/libjpeg/build/djpeg.o: In function `main':
djpeg.c:(.text+0xe9f): multiple definition of `main'
enclave/libjpeg/build/jpegtran.o:jpegtran.c:(.text+0xb66): first defined here
enclave/libjpeg/build/cjpeg.o: In function `main':
cjpeg.c:(.text+0xd0c): multiple definition of `main'
enclave/libjpeg/build/jpegtran.o:jpegtran.c:(.text+0xb66): first defined here
```
_solution_: some time some projects create multiple object files containing the same defination of symbol, especially those build with `libtool` in `.deps` or `.lib` directory. Remove the later object file.

* relocation error; recompile with -fPIC
```
/usr/bin/ld: /usr/lib/gcc/x86_64-linux-gnu/5/../../../x86_64-linux-gnu/libc.a(libc-start.o): relocation R_X86_64_32 against `_dl_starting_up' can not be used when making a shared object; recompile with -fPIC
/usr/lib/gcc/x86_64-linux-gnu/5/../../../x86_64-linux-gnu/libc.a: error adding symbols: Bad value
```
_solution_: compile the depency library with `CFLAGS="-fPIC"` to build position independent code.

Caveat:-
1. Porpoise will work out of the box only for those applications that use the system calls for which we have added support. If it invokes a system call at runtime for which we have not added support, it will not work.
2. Porpoise is build on musl-libc, so if an application uses any symbol from libc which is not present in musl-libc, the application will not link and gives _undefined reference error_
3. Porpoise doesn't support dynamic linking as SGX doesn't support dynamic loading of code.

In case of any queries contact:

```
Kripa Shanker
Department of Computer Science and Automation
Indian Institute of Science
Bengaluru - 560012
India
email: kripashanker@iisc.ac.in
```
