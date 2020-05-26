/************************************************
 * shim layer copy the required arguments of systemcall
 * outside the enclave and leave encalve to execute system
 * call outside enclave
 * ************************************************/

#ifndef _SHIM_LAYER_H_
#define _SHIM_LAYER_H_

#include "enclave_t.h"
#include <sgx_thread.h>
#include <signal.h>
#include <pthread.h>
#include <stdio.h>

//syscall arg buffer size
#define BUFFER_SIZE 4098


extern pthread_spinlock_t syscall_arg_table_lock;
extern pthread_spinlock_t object_table_lock;
extern sgx_thread_mutex_t syscall_mutex;
extern volatile uint32_t syscall_lock;
extern void *signo;

//Note: MAX_NO_OF_THREADS also change in app/shim_layer/shim_layer.cpp
#define MAX_NO_OF_THREADS 16
extern struct syscall_arg_t syscall_arg_table[MAX_NO_OF_THREADS];

extern struct sigaction handler_list[65];

#if defined(__cplusplus)
extern "C" {
#endif

int main(int argc, char **argv);

int __libc_start_main(int (*)(), int, char **,
	void (*)(), void(*)(), void(*)());

long __syscall_wrap(long n, long a1, long a2, long a3, long a4, long a5, long a6);
int send_user(void *user_addr,const void *encl_addr, int len, int prot);
int recv_user(const void *user_addr, void *encl_addr, int len, int prot);

void *sbrk(intptr_t increment);
void inside_sig_handler(int sig);
void *malloc_user(size_t size);
void free_user(void *ptr);

#if defined(__cplusplus)
}
#endif

#endif /* !_SHIM_LAYER_H_ */
