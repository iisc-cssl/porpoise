#ifndef _SHIM_LAYER_H
#define _SHIM_LAYER_H

#include "enclave_u.h"
#include <signal.h>

#include "sgx_perf.h"

extern sgx_enclave_id_t enclave_id;    /* global enclave id */
extern struct syscall_arg_t syscall_arg_table[];
extern long *signo;
extern void* sgx_sig_handler_ptr;

void outside_sig_handler(int signum, siginfo_t* siginfo, void *priv);
struct out_start_routine_arg_t{
	pid_t tid;
	void *arg;
};

#if defined(__cplusplus)
extern "C" {
#endif

void *out_start_routine(void *arg);
extern int ARGC;
extern char **ARGV; 
#if defined(__cplusplus)
}
#endif

#endif /* !_SHIM_LAYER_H */
