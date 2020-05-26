#include <stdio.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#include "shim_layer.h"
#include "sgx_perf.h"

char file_name[] = "results/perf.result" ;
FILE *fp = NULL;
struct sgx_perf_ecall_ocall {
	unsigned long long ecall_cycles;
	unsigned long long ocall_cycles;
	struct timeval ecall_time;
	struct timeval ocall_time;
	int count;
} perf;

static __inline__ unsigned long long rdtsc(void)
{
    unsigned hi, lo;
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
    return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}

void perf_ecall(){


	struct timeval start = {0};
	struct timeval stop = {0};
	timerclear(&start);
	timerclear(&stop);
	
	
	gettimeofday(&start, NULL);

	unsigned long long begin = rdtsc();
	
	int i = 0;
	for( i = 0; i< perf.count; i++){

		int ret = -1;
		sgx_status_t status = ecall_empty(enclave_id, &ret);
		assert(status == SGX_SUCCESS);
		assert(ret == 0);
	}


	unsigned long long end = rdtsc();
	
	perf.ecall_cycles += end - begin;
	
	gettimeofday(&stop, NULL);

	timersub(&stop, &start, &perf.ecall_time);


}

void perf_ocall(){

	struct timeval start = {0};
	struct timeval stop = {0};
	timerclear(&start);
	timerclear(&stop);
	

	gettimeofday(&start, NULL);

	int ret = -1;
	unsigned long long begin = rdtsc();

	sgx_status_t status = ecall_perf_ocall(enclave_id, &ret, perf.count);
	
	unsigned long long end = rdtsc();
	
	perf.ocall_cycles += end - begin;

	assert(status == SGX_SUCCESS);
	assert(ret == 0);

	gettimeofday(&stop, NULL);

	timersub(&stop, &start, &perf.ocall_time);

}

FILE* perf_open_file(){
	
	char *new_name = (char*) malloc(sizeof(file_name) + 4);

	int i;
	for(i = 0; i < 9999; i++){
		sprintf(new_name, "results/%s.%d.txt", ARGV[0], i);
		struct stat buf;
		if(stat(new_name, &buf) != 0)
			return fopen(new_name, "w");
	}
}

void perf_setup(){
	perf.count = 10 * 1000 * 1000;
	perf.ecall_cycles = 0;
	perf.ocall_cycles = 0;
	
	fp = perf_open_file();

	if(fp == NULL){
		perror("(perf_setup)");
		exit(-1);
	}
}

void perf_cleanup(){
	
	time_t t = time(NULL);
	fprintf(fp, "\nTest time : %s", asctime(localtime(&t)));

	fprintf(fp, "\n=== Ecall Summary ===\n");
	fprintf(fp, "Ecall iteration		: %d\n", perf.count);
	fprintf(fp, "Total ecall time	: %ld.%ld s\n", perf.ecall_time.tv_sec,
			perf.ecall_time.tv_usec);
	fprintf(fp, "Avg ecall time		: %f micro seconds\n", (double)(perf.ecall_time.tv_sec *1000 * 1000  + perf.ecall_time.tv_usec) / perf.count);
	fprintf(fp, "Ecall cycles		: %llu\n", perf.ecall_cycles);
	fprintf(fp, "Avg ecall cycles	: %llu\n", perf.ecall_cycles/perf.count);
	
	fprintf(fp, "\n=== Ocall Summary ===\n");
	fprintf(fp, "Ocall iteration		: %d\n", perf.count);
	fprintf(fp, "Total Ocall time	: %ld.%ld\n", perf.ocall_time.tv_sec,
			perf.ocall_time.tv_usec);
	fprintf(fp, "Avg Ocall time		: %f micro seconds\n", (double)(perf.ocall_time.tv_sec * 1000 * 1000 + perf.ocall_time.tv_usec) / perf.count);
	fprintf(fp, "Ocall cycles		: %llu\n", perf.ocall_cycles);
	fprintf(fp, "Avg ocall cycles	: %llu\n", perf.ocall_cycles/perf.count);
	
	fclose(fp);
}

void perf_run(){
	perf_setup();

	perf_ecall();
	perf_ocall();
	
	perf_cleanup();
}
