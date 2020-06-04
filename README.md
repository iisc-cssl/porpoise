Porpoise

```
$ sh build.sh
$ make all
$ make test
```
# Porting additional application to SGX with Porpoise:-
1. Compile the application and dependent libraries with position independent code.
```
$ ./configure CFLAS="-fPIC -D_FORTIFY_SOURCE=0" --prefix=$PWD/install
$ make
```

2. Create new Makefile with filename Makefile.<application name>
```
```

6. Build the enclave with Porpoise using Makefile.<application name>
7. Run the application using ./<application name>



libevent header file
