
/*
 * app/shim_layer/shim_layer.cpp
 *
 * Kripa Shanker <kripashanker@iisc.ac.in>
 *
 * This is user part of shim layer.
 *
 * Shim layer creates the enclave and initialize the buffers
 * which will be used used by enclave to copy arguments of system calls
 * outside the enclave.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <sched.h>
#include <libgen.h>
#include <sys/times.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <syscall.h>
#include <unistd.h>
#include <sys/syscall.h>

#include "sgx_urts.h"					// for sgx_create_enclave()

#include "shim_layer.h"

#define BUF_SIZE 8192 * 10

#define MAX_NO_OF_THREADS 16
struct syscall_arg_t syscall_arg_table[MAX_NO_OF_THREADS];

long *signo = NULL;
void *sgx_sig_handler_ptr = NULL;

sgx_enclave_id_t enclave_id = NULL;

char **ARGV;
int ARGC;

static int shim_init()
{
	sgx_status_t enclave_created, ecall_success;
	sgx_launch_token_t launch_token;
	int launch_token_updated;
	int ret;


	int i;
	for(i = 0; i < MAX_NO_OF_THREADS; i++){
		syscall_arg_table[i].busy = 0;
		syscall_arg_table[i].err_no = (long*)malloc(BUF_SIZE);
		syscall_arg_table[i].arg0 = (long*)malloc(BUF_SIZE);
		syscall_arg_table[i].arg1 = (long*)malloc(BUF_SIZE);
		syscall_arg_table[i].arg2 = (long*)malloc(BUF_SIZE);
		syscall_arg_table[i].arg3 = (long*)malloc(BUF_SIZE);
		syscall_arg_table[i].arg4 = (long*)malloc(BUF_SIZE);
		syscall_arg_table[i].arg5 = (long*)malloc(BUF_SIZE);
		syscall_arg_table[i].arg6 = (long*)malloc(BUF_SIZE);
		syscall_arg_table[i].arg7 = (long*)malloc(BUF_SIZE);
		syscall_arg_table[i].arg8 = (long*)malloc(BUF_SIZE);
	}


	signo = (long*)malloc(sizeof(long));

	memset(&launch_token, 0, sizeof(sgx_launch_token_t));

	/* create enclave */
	char signed_enclave_name[200] = {NULL};
	sprintf(signed_enclave_name,"enclave.%s.signed.so",basename(ARGV[0]));
	printf("signed_enclave_name = %s\n", signed_enclave_name);
	printf("SGX_DEBUF_FLAG = %d\n", SGX_DEBUG_FLAG);
	enclave_created = sgx_create_enclave(signed_enclave_name, SGX_DEBUG_FLAG, &launch_token,
					&launch_token_updated, &enclave_id, NULL);
	//assert(status == SGX_SUCCESS);
	if (enclave_created != SGX_SUCCESS){
		printf("ERR: status = %x run: systemctl status aesmd.service\n",enclave_created);
		return -1;
	}

	ecall_success = ecall_init_transfer(enclave_id, &ret, syscall_arg_table, signo);
	assert(ecall_success == SGX_SUCCESS);

	/* for sig_handler function pointer */
	struct sigaction act_old;
	sigaction(SIGFPE, NULL, &act_old);
	sgx_sig_handler_ptr =  (void*)act_old.sa_sigaction;

	printf("(user) MAX_NO_OF_THREADS %d\n", MAX_NO_OF_THREADS);
	return 0;
}

static int shim_terminate()
{
	/* Destroy the enclave */
	sgx_status_t enclave_destroyed;
	enclave_destroyed = sgx_destroy_enclave(enclave_id);
	if(enclave_destroyed != SGX_SUCCESS){
		printf("ERR: unable to destroy enclave "
			"enclave_id %ld, error_code: %x\n", enclave_id, enclave_destroyed);
		return -1;
	}

	return 0;
}

void outside_sig_handler(int signum, siginfo_t* siginfo, void *priv)
{
	*signo = signum;
	void (*fun_ptr)(int, siginfo_t*, void *) = 
		(void (*) (int, siginfo_t*, void *)) sgx_sig_handler_ptr;
	//(*fun_ptr)(signum, siginfo, priv);
	return;

}

void* out_start_routine(void *arg){

	int ecall_status = ecall_start_routine(enclave_id, arg);
	if(ecall_status != SGX_SUCCESS){
		printf("ecall_start_routine error %x\n", ecall_status);
		exit(EXIT_FAILURE);
	}

	return NULL;

	
}

/*
#ifdef PERFORMANCE_ANALYSIS
struct performance_metrics* performance = NULL;
#endif
*/

int main(int argc, char *argv[])
{
	ARGC = argc;
	ARGV = argv;
#ifdef SGX_STATISTICS
	struct rusage main_start, main_end;

	if(getrusage(RUSAGE_SELF, &main_start))
		perror("(main_start)");
#endif

	/*
	#ifdef PERFORMANCE_ANALYSIS
	performance = (struct performance_metrics*) malloc(sizeof(struct performance_metrics));
	unsigned int t1=0,t2=0,t3=0,t4=0;
	uint64_t start = 0, end = 0;
	struct timespec start_rt, end_rt;
	performance->no_of_syscalls = 0;
	clock_gettime(CLOCK_REALTIME, &start_rt);
	performance->total_time_real_start = start_rt.tv_sec +
	  start_rt.tv_nsec / BILLION;
	RDTSC_START(t1,t2);
	#endif
	*/

#ifdef SGX_STATISTICS
	struct timeval shim_start = {0};
	struct timeval shim_stop = {0};
	timerclear(&shim_start);
	timerclear(&shim_stop);
	
	if(gettimeofday(&shim_start, NULL))
		perror("(main)");

#endif
	if(shim_init() != 0){
		printf("unable to initialize shim layer\n");
		exit(0);
	}

#ifdef SGX_STATISTICS
	if(gettimeofday(&shim_stop, NULL))
		perror("(main)");
	{
	struct timeval res;
	timersub(&shim_stop, &shim_start, &res);
	sgx_perf_add_shim_time(res);
	}

#endif
	/*
	#ifdef PERFORMANCE_ANALYSIS
	RDTSC_STOP(t3,t4);
	clock_gettime(CLOCK_REALTIME, &end_rt);
	start = ( ((long long)t1 << 32) | t2 );
	end = ( ((long long)t3 << 32) | t4 );
	performance->shim_init_time = end - start;
	performance->total_time_start = start;
	performance->shim_init_real = (end_rt.tv_sec - start_rt.tv_sec) +
	  (end_rt.tv_nsec - start_rt.tv_nsec) / BILLION;
	#endif
	*/
	printf("Welcome to sample application\n");

	int returned_value;
	sgx_status_t ecall_success;

#ifdef PERF
	perf_run();
#else
	ecall_success = ecall_shim_main(enclave_id, &returned_value, argc, argv);
	assert(ecall_success == SGX_SUCCESS);
#endif

#ifdef SGX_STATISTICS
	timerclear(&shim_start);
	timerclear(&shim_stop);
	
	if(gettimeofday(&shim_start, NULL))
		perror("(main)");

#endif

	if(shim_terminate() != 0){
		printf("unable to terminate shim layer\n");
		exit(0);
	}

#ifdef SGX_STATISTICS
	if(gettimeofday(&shim_stop, NULL))
		perror("(main)");
	
	struct timeval res = {0};
	timersub(&shim_stop, &shim_start, &res);

	sgx_perf_add_shim_time(res);
#endif

#ifdef SGX_STATISTICS
	sgx_perf_terminate();
#endif

	return returned_value;
}

int ocall_empty(){
	return 0;
}
