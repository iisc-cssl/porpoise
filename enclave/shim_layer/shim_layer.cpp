/*
 * enclave/shim_layer/shim_layer.cpp
 *
 * Kripa Shanker <kripashanker@iisc.ac.in>
 *
 * Shim layer transition processor from enclave mode to user mode,
 * whenever there is need of system call using __syscall_wrap() function.
 *
 */

#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <pthread.h>
#include "shim_layer.h"
#include <syscall.h>
#include <unistd.h>
#include <string.h>


#include "sgx_trts_exception.h"
#include "sgx_trts.h"

void *signo = NULL;

sgx_thread_mutex_t syscall_mutex;
volatile uint32_t syscall_lock;


struct sigaction handler_list[65];
/*
 * inside_sig_handler()
 * signal handler inside will call the handling function
 *
 */
int inside_sig_handler(sgx_exception_info_t *info)//not being invocked
{
	int sig = *(int*)signo;
	void (*fun_ptr)(int) = handler_list[sig].sa_handler;
	if(*fun_ptr == NULL){//then the handler function is in sa_sigaction
		void (*fun_ptr2 )(int, siginfo_t*, void *) = handler_list[sig].sa_sigaction;
		(*fun_ptr2)(sig, NULL, NULL);
		return 0;
	}
	(*fun_ptr)(sig);
	return 0;
}

/* syscall_arg_table */
struct syscall_arg_t syscall_arg_table[MAX_NO_OF_THREADS];

/*
 * initialize pointers to buffers, where buffer are in user memory
 */
int ecall_init_transfer(struct syscall_arg_t syscall_arg_table_out[], void *buf7)
{
	memcpy(syscall_arg_table, syscall_arg_table_out, sizeof(struct syscall_arg_t) * MAX_NO_OF_THREADS);

	signo = buf7;
	sgx_register_exception_handler(1, inside_sig_handler);
	
	return 0;
}

void ecall_start_routine(void *arg){
	struct clone_arg_t *clone_arg = (struct clone_arg_t*)arg;
	void *(*start_routine)(void*) = clone_arg->start_routine; //TODO
	(*start_routine)(clone_arg->arg);
}

int ecall_perf_ocall(int count){
	
	int i = 0;
	
	for(i = 0; i < count; i++){
		int ret = -1;
		sgx_status_t status = ocall_empty(&ret);
		assert(status == SGX_SUCCESS);
		assert(ret == 0);
	}
	return 0;
}

pthread_mutex_t job_lock;

void *print_kripa(void *arg){

	pthread_mutex_lock(&job_lock);

	char str[1024];
	pid_t tid = syscall(SYS_gettid);
	sprintf(str, "begin: print_kripa thread %d\n", tid);
	write(1, str, strlen(str));
	
	int i;
	for(i = 0; i < 1000000000; i++);

	sprintf(str, "end: print_kripa thread %d\n", tid);
	write(1, str, strlen(str));
	
	pthread_mutex_unlock(&job_lock);
	return NULL;
	
}

int main1(){
	printf("(enclave) MAX_NO_OF_THREADS %d\n", MAX_NO_OF_THREADS);
	pthread_spin_init(&syscall_arg_table_lock, PTHREAD_PROCESS_PRIVATE);
	pthread_spin_init(&object_table_lock, PTHREAD_PROCESS_PRIVATE);
	printf("main1: begin\n");

	pthread_mutex_init(&job_lock, NULL);

	pthread_t tid1, tid2, tid3, tid4, tid5, tid6, tid7;
	pthread_create(&tid1, NULL, print_kripa, NULL);
	pthread_create(&tid2, NULL, print_kripa, NULL);
	pthread_create(&tid3, NULL, print_kripa, NULL);
	pthread_create(&tid4, NULL, print_kripa, NULL);
	pthread_create(&tid5, NULL, print_kripa, NULL);
	pthread_create(&tid6, NULL, print_kripa, NULL);
	pthread_create(&tid7, NULL, print_kripa, NULL);
	print_kripa(NULL);
	pthread_join(tid1, NULL);
	pthread_join(tid2, NULL);
	pthread_join(tid3, NULL);
	pthread_join(tid4, NULL);
	pthread_join(tid5, NULL);
	pthread_join(tid6, NULL);
	pthread_join(tid7, NULL);
	printf("main1: end\n");
	return 0;
}

int ecall_shim_main(int argc, char **argv){

	void *ptr = malloc(10);
	__libc_start_main((int(*)())main, argc, argv, NULL, NULL, 0);
	return -1;
}

void ecall_sig_handler(int signum){
	printf("ecall_sig_handler\n");
}

int ecall_empty(){
	return 0;
}
