applications = python openssl memcached h2o bzip2

all: $(applications)

test: python_test openssl_test memcached_test h2o_test bzip2_test


python: | enclave/python/build/python enclave.python.signed.so
	@echo Building $@ enclave
	$(MAKE) -f Makefile.$@

openssl: | enclave/openssl/apps/openssl enclave.openssl.signed.so
	@echo Building $@ enclave
	$(MAKE) -f Makefile.$@

bzip2: | enclave/bzip2/bzip2 enclave.bzip2.signed.so
	@echo Building $@ enclave
	@$(MAKE) -f Makefile.$@

memcached: | enclave/memcached/memcached enclave.memcached.signed.so
	@echo Building $@ enclave
	$(MAKE) -f Makefile.$@

# h2o require openssl to be installed in /usr/local/ssl with -fPIC flags
h2o: | enclave/h2o/build/h2o enclave.h2o.signed.so
	@echo Building $@ enclave
	$(MAKE) -f Makefile.$@

njs: | enclave/njs/build/njs enclave.njs.signed.so
	@echo Building $@ enclave
	$(MAKE) -f Makefile.$@

enclave/python/build/python: enclave/python/Programs/python.c
	@echo Building $@
	(cd enclave/python; $(SHELL) build.sh)

enclave/openssl/apps/openssl: enclave/openssl/apps/openssl.c
	@echo Building $@
	(cd enclave/openssl; $(SHELL) build.sh)
enclave/memcached/memcached: enclave/memcached/*.c enclave/libevent/_install/lib/libevent.a
	@echo Building $@
	(cd enclave/memcached; $(SHELL) build.sh)

enclave/bzip2/bzip2: enclave/bzip2/*.c 
	@echo Building $@
	(cd enclave/bzip2; $(SHELL) build.sh)

enclave/h2o/build/h2o:
	@echo Building $@
	(cd enclave/h2o; $(SHELL) build.sh)

enclave/njs/build/njs:
	@echo Building $@
	(cd enclave/njs; $(SHELL) build.sh)

enclave.python.signed.so:
	$(MAKE) -f Makefile.python

enclave.openssl.signed.so:
	$(MAKE) -f Makefile.openssl

enclave.memcached.signed.so:
	$(MAKE) -f Makefile.memcached

enclave.bzip2.signed.so:
	$(MAKE) -f Makefile.bzip2

enclave.h2o.signed.so:
	$(MAKE) -f Makefile.h2o

enclave.njs.signed.so:
	$(MAKE) -f Makefile.njs

python_test: python
	@PYTHONPATH=${PWD}/enclave/python/Modules:${PWD}/enclave/python/Lib:${PWD}/enclave/python ./python  -c 'x="Keep Calm Stay Strong"; print(x)' 2>/dev/null | grep "Keep Calm Stay Strong" > out.txt; \
	diff  out.txt tests/python_expected_output.txt ; \
	if [ $$? -ne 0 ] ; then echo  "\033[1;31m $@ TEST FAILED \033[0m"; \
	else echo  "\033[1;32m $@ TEST PASS \033[0m"; \
	fi
	@rm out.txt

openssl_test: openssl
	@./openssl dgst -sha256 tests/openssl_sha256_input.txt > out.txt 2>/dev/null;\
	diff  out.txt tests/openssl_sha256_expected_output.txt; \
	if [ $$? -ne 0 ] ; then echo  "\033[1;31m $@ TEST FAILED \033[0m"; \
	else echo  "\033[1;32m $@ TEST PASS \033[0m"; \
	fi
	@rm out.txt
memcached_test: memcached
	@./tests/memcached_test 2> /dev/null > out.txt;\
	diff  out.txt tests/memcached_expected_output.txt;\
	if [ $$? -ne 0 ] ; then echo  "\033[1;31m $@ TEST FAILED \033[0m"; \
	else echo  "\033[1;32m $@ TEST PASS \033[0m"; \
	fi
	@rm out.txt


bzip2_test: bzip2
	@./bzip2 -zk tests/bzip2_expected_output.txt 2>/dev/null >/dev/null;\
	mv tests/bzip2_expected_output.txt.bz2 out.txt.bz2;\
	./bzip2 -d out.txt.bz2 2> /dev/null > /dev/null; \
	diff  out.txt tests/bzip2_expected_output.txt;\
	if [ $$? -ne 0 ] ; then echo  "\033[1;31m $@ TEST FAILED \033[0m"; \
	else echo  "\033[1;32m $@ TEST PASS \033[0m"; \
	fi
	@rm out.txt

h2o_test: | h2o
	@ ./tests/h2o_test 2> /dev/null > out.txt;\
	diff  out.txt tests/h2o_expected_output.txt;
	@if [ $$? -ne 0 ] ; then echo  "\033[1;31m $@ TEST FAILED \033[0m"; \
	else echo  "\033[1;32m $@ TEST PASS \033[0m"; \
	fi
	@rm out.txt

njs_test: | njs
	@ ./njs tests/njs_input.js 2>/dev/null | grep "Hello World!" > out.txt;\
	diff  out.txt tests/njs_expected_output.txt;\
	if [ $$? -ne 0 ] ; then echo  "\033[1;31m $@ TEST FAILED \033[0m"; \
	else echo  "\033[1;32m $@ TEST PASS \033[0m"; \
	fi
	@rm out.txt
		

#enclave.so:
#$(applications): enclave.so
#	$(MAKE) -f Makefile.$@

clean:
	$(MAKE) -f Makefile.python clean
	-rm -f $(applications)
	-rm *.bz2 enclave.*.so
enclave/libevent/_install/lib/libevent.a: |
	(cd enclave/libevent; bash build.sh)
git_clean_all: clean
	-(cd enclave/python/build; make clean; rm python)
	(cd enclave/openssl; make clean)
	(cd enclave/memcached; make clean)
	(cd enclave/h2o/build; make clean)
	(cd enclave/bzip2; make clean)
	(cd enclave/libevent; make clean)
	(cd enclave/musl/build; make clean)

.PHONY: clean

BUILD_PRINT = \033[1;34mBuilding $<\033[0m
build_msg:
	echo  "\033[1;32m python_test TEST PASS \033[0m"
	@echo  "$(BUILD_PRINT)\n$(COMPILE_cpp)\n$(COMPILE_cpp_OUT)"
	@printf "normal text - `tput bold`bold text`tput sgr0`"
	echo ${PWD}
