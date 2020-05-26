######## SGX SDK Settings ########

SGX_SDK ?= /opt/intel/sgxsdk
SGX_MODE ?= HW
SGX_ARCH ?= x64
SGX_DEBUG ?= 1

CC = gcc
MUSL_SYSTEM := enclave/musl/install
MUSL_ENCLAVE := enclave/musl/build# location of enclave musl

user_application_source := $(wildcard app/$(application_name)/*.cpp)
user_application_objects := $(user_application_source:.cpp=.o)

user_shim_layer_source := $(wildcard app/shim_layer/*.cpp)
user_shim_layer_objects := $(user_shim_layer_source:.cpp=.o)

user_source := $(user_application_source) $(user_shim_layer_source)
user_objects := $(user_shim_layer_objects) $(user_application_objects)


#enclave_application_objects :=
enclave_application_objects ?= $(shell find $(native_application_location) -name "*.o")

#enclave_application_source := $(enclave_application_objects:.o=.c)

enclave_shim_layer_source := recv.cpp  send.cpp  shim_layer.cpp  syscall_wrap.cpp malloc_user.cpp
enclave_shim_layer_source := $(addprefix enclave/shim_layer/,$(enclave_shim_layer_source))
enclave_shim_layer_objects := $(enclave_shim_layer_source:.cpp=.o)

enclave_source := $(enclave_shim_layer_source) $(enclave_application_source)
enclave_objects := $(enclave_shim_layer_objects) $(enclave_application_objects)

ifeq ($(shell getconf LONG_BIT), 32)
	SGX_ARCH := x86
else ifeq ($(findstring -m32, $(CXXFLAGS)), -m32)
	SGX_ARCH := x86
endif

ifeq ($(SGX_ARCH), x86)
	SGX_COMMON_CFLAGS := -m32
	SGX_LIBRARY_PATH := $(SGX_SDK)/lib
	SGX_ENCLAVE_SIGNER := $(SGX_SDK)/bin/x86/sgx_sign
	SGX_EDGER8R := $(SGX_SDK)/bin/x86/sgx_edger8r
else
	SGX_COMMON_CFLAGS := -m64
	SGX_LIBRARY_PATH := $(SGX_SDK)/lib64 # we are using our custom build
	SGX_ENCLAVE_SIGNER := $(SGX_SDK)/bin/x64/sgx_sign
	SGX_EDGER8R := $(SGX_SDK)/bin/x64/sgx_edger8r
endif

ifeq ($(SGX_DEBUG), 1)
ifeq ($(SGX_PRERELEASE), 1)
$(error Cannot set SGX_DEBUG and SGX_PRERELEASE at the same time!!)
endif
endif

ifeq ($(SGX_DEBUG), 1)
        SGX_COMMON_CFLAGS += -O0 -g
else
        SGX_COMMON_CFLAGS += -O0
endif

ifeq ($(SGX_PERF), 1)
	SGX_COMMON_CFLAGS += -DSGX_PERF
endif

ifeq ($(application_name), perf)
	SGX_COMMON_CFLAGS += -DPERF
endif

######## App Settings ########

ifneq ($(SGX_MODE), HW)
	urts_library_name := sgx_urts_sim
else
	urts_library_name := sgx_urts
endif

ifeq ($(SGX_DEBUG), 1)
        user_application_c_flags += -DDEBUG -UNDEBUG -UEDEBUG
else ifeq ($(SGX_PRERELEASE), 1)
        user_application_c_flags += -DNDEBUG -DEDEBUG -UDEBUG
else
        user_application_c_flags += -DNDEBUG -UEDEBUG -UDEBUG
endif


ifneq ($(SGX_MODE), HW)
	user_application_link_flags += -lsgx_uae_service_sim
else
	user_application_link_flags += -lsgx_uae_service
endif

user_application_include_paths := -I$(SGX_SDK)/include -I$(MUSL_SYSTEM)/include \
	-Iapp -Iapp/shim_layer -Iinclude
user_application_c_flags := $(SGX_COMMON_CFLAGS) -fPIC -Wno-attributes $(user_application_include_paths)
user_application_cpp_flags := $(user_application_c_flags) -nostdinc -std=c++11
user_application_link_flags := $(SGX_COMMON_CFLAGS) -L$(SGX_LIBRARY_PATH)\
	-l$(urts_library_name) -pthread


######## Enclave Settings ########

ifneq ($(SGX_MODE), HW)
	trts_library_name := sgx_trts_sim
	service_library_name := sgx_tservice_sim
else
	trts_library_name := sgx_trts
	service_library_name := sgx_tservice
endif
crypto_library_name := sgx_tcrypto

CC_BELOW_4_9 := $(shell expr "`$(CC) -dumpversion`" \< "4.9")
ifeq ($(CC_BELOW_4_9), 1)
	enclave_c_flags := $(SGX_COMMON_CFLAGS) -nostdinc -fvisibility=hidden -fpie -ffunction-sections -fdata-sections -fstack-protector
else
	enclave_c_flags := $(SGX_COMMON_CFLAGS) -nostdinc -fvisibility=hidden -fpie -ffunction-sections -fdata-sections -fstack-protector-strong
endif

enclave_cpp_files := $(wildcard enclave/test/*.cpp) $(wildcard enclave/sample_app/*.cpp) \
	$(wildcard enclave/shim_layer/*.cpp)

enclave_include_paths := -I$(SGX_SDK)/include \
	-I$(MUSL_SYSTEM)/include \
	-I$(SGX_SDK)/include/tlibc\
	-I$(SGX_SDK)/include/libcxx \
	-Ienclave \
	-Ienclave/shim_layer \
	-Ienclave/$(application_name)/include


enclave_c_flags += $(enclave_include_paths)
enclave_cpp_flags := $(enclave_c_flags) -std=c++11 -nostdinc++

enclave_link_flags := $(SGX_COMMON_CFLAGS)\
	-Wl,--no-undefined\
	-nodefaultlibs\
	-nostartfiles\
	-L$(SGX_LIBRARY_PATH) \
	-L$(MUSL_ENCLAVE)/lib \
	-Lenclave/libevent/.libs\
	-Lenclave/openssl\
	-Lenclave/pcre/.libs\
	-Lenclave/shim_layer \
	-Lenclave/lib\
	-Wl,--whole-archive\
		-l$(trts_library_name)\
	-Wl,--no-whole-archive \
	-Wl,--start-group\
		enclave/lib/libm.a\
		-lsgx_tstdc \
		-l:libc.a\
		-lsgx_tcxx \
		-l$(crypto_library_name) \
		-l$(service_library_name) \
		-l:libcrypto.a \
		-l:libssl.a\
		-l:libz.a\
		-l:libbz2.a\
		-l:libevent.a\
		-l:libncurses.a\
		-l:libreadline.a\
		-l:libpcre.a\
	-Wl,--end-group \
	-Wl,-Bstatic \
	-Wl,-Bsymbolic \
	-Wl,-pie,-eenclave_entry \
	-Wl,--export-dynamic  \
	-Wl,--defsym,__ImageBase=0\
	-Wl,--gc-sections \
	-Wl,--version-script=enclave/enclave.lds\
	-Wl,--no-undefined \

enclave_name := enclave.$(application_name).so
signed_enclave_name := enclave.$(application_name).signed.so
enclave_config_file := enclave/enclave.config.xml

ifeq ($(SGX_MODE), HW)
ifeq ($(SGX_DEBUG), 1)
	build_mode = HW_DEBUG
else ifeq ($(SGX_PRERELEASE), 1)
	build_mode = HW_PRERELEASE
else
	build_mode = HW_RELEASE
endif
else
ifeq ($(SGX_DEBUG), 1)
	build_mode = SIM_DEBUG
else ifeq ($(SGX_PRERELEASE), 1)
	build_mode = SIM_PRERELEASE
else
	build_mode = SIM_RELEASE
endif
endif


.PHONY: all run

ifeq ($(build_mode), HW_RELEASE)
all: .config_$(build_mode)_$(SGX_ARCH) $(application_name) $(enclave_name)
	@echo "The project has been built in release hardware mode."
	@echo "Please sign the $(enclave_name) first with your signing key before you run the $(application_name) to launch and access the enclave."
	@echo "To sign the enclave use the command:"
	@echo "   $(SGX_ENCLAVE_SIGNER) sign -key <your key> -enclave $(enclave_name) -out <$(signed_enclave_name)> -config $(enclave_config_file)"
	@echo "You can also sign the enclave using an external signing tool."
	@echo "To build the project in simulation mode set SGX_MODE=SIM. To build the project in prerelease mode set SGX_PRERELEASE=1 and SGX_MODE=HW."
else
all: .config_$(build_mode)_$(SGX_ARCH) $(application_name) $(signed_enclave_name)
ifeq ($(build_mode), HW_DEBUG)
	@echo "The project has been built in debug hardware mode."
else ifeq ($(build_mode), SIM_DEBUG)
	@echo "The project has been built in debug simulation mode."
else ifeq ($(build_mode), HW_PRERELEASE)
	@echo "The project has been built in pre-release hardware mode."
else ifeq ($(build_mode), SIM_PRERELEASE)
	@echo "The project has been built in pre-release simulation mode."
else
	@echo "The project has been built in release simulation mode."
endif
endif

run: all
ifneq ($(build_mode), HW_RELEASE)
	@$(CURDIR)/$(application_name)
	@echo "RUN  =>  $(application_name) [$(SGX_MODE)|$(SGX_ARCH), OK]"
endif

######## App Objects ########

app/enclave_u.c: $(SGX_EDGER8R) enclave/enclave.edl
	@cd app && $(SGX_EDGER8R) --untrusted ../enclave/enclave.edl --search-path ../enclave --search-path $(SGX_SDK)/include
	@echo "GEN  =>  $@"

app/enclave_u.o: app/enclave_u.c
	@$(CC) $(user_application_c_flags) -c $< -o $@
	@echo "CC   <=  $<"

app/%.o: app/%.cpp
	$(CXX) $(user_application_cpp_flags) -c $< -o $@
	@echo "CXX  <=  $<"

app/%.o: app/%.cxx
	@$(CXX) $(user_application_cpp_flags) -c $< -o $@
	@echo "CXX  <=  $<"

$(application_name): app/enclave_u.o $(user_objects) $(perf_objects)
	@$(CXX) $^ -o $@ $(user_application_link_flags)
	@echo "LINK =>  $@"

.config_$(build_mode)_$(SGX_ARCH):
	@echo .config_* $(application_name) $(enclave_name) $(signed_enclave_name) $(user_objects) app/enclave_u.* $(enclave_shim_layer_objects) enclave/enclave_t.*

	@rm -vf .config_* $(application_name) $(enclave_name) $(signed_enclave_name) $(user_objects) app/enclave_u.* $(enclave_shim_layer_objects) enclave/enclave_t.*
#SGX_HACK dont want to build apllication objects$(enclave_application_objects)
	@touch .config_$(build_mode)_$(SGX_ARCH)


######## Enclave Objects ########
$(native_application_executable):
	@cd $(native_application_location) && $(MAKE) $(make_target)

enclave/enclave_t.c: $(SGX_EDGER8R) enclave/enclave.edl
	@cd enclave && $(SGX_EDGER8R) --trusted ../enclave/enclave.edl --search-path ../enclave --search-path $(SGX_SDK)/include
	@echo "GEN  =>  $@"

enclave/enclave_t.o: enclave/enclave_t.c
	$(CC) $(enclave_c_flags) -c $< -o $@
	@echo "CC   <=  $<"

enclave/%.o: enclave/%.cpp
	@$(CXX) $(enclave_cpp_flags) -c $< -o $@
	@echo "CXX  <=  $<"

enclave/%.o: enclave/%.c
	@$(CXX) $(enclave_cpp_flags) -c $< -o $@
	@echo "CXX  <=  $<"

enclave/%.o: enclave/%.cxx
	@$(CXX) $(enclave_cpp_flags) -c $< -o $@
	@echo "CXX  <=  $<"

$(enclave_name): enclave/enclave_t.o $(enclave_shim_layer_objects)
	@$(CXX) $^ $(enclave_application_objects) -o $@ $(enclave_link_flags)
	@echo "LINK =>  $@"

$(signed_enclave_name): $(enclave_name)
	$(SGX_ENCLAVE_SIGNER) sign -key enclave/enclave_private.pem -enclave $(enclave_name) -out $@ -config $(enclave_config_file)
	@echo "SIGN =>  $@"

clean:
	@rm -vf  $(application_name) $(enclave_name) $(signed_enclave_name) $(user_objects) $(perf_objects) app/enclave_u.* $(enclave_shim_layer_objects) enclave/enclave_t.* #SGX_HACK deteles all enclave objects .config_*

build_msg:
	@echo $(enclave_application_objects)
