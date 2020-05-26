#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "sgx_perf.h"

extern int ARGC;
extern char **ARGV;
FILE *fp = NULL;
char file_name[100];

static struct rusage main_begin;
static struct rusage main_end;
#define TEST_ECALL 0
#define TEST_OCALL 1

/* Macro Benchmark */
struct sgx_perf_t {
	long ocall_count;
	long ecall_count;
	long syscall_count[320];
	struct timeval main;
	struct timeval shim;
	struct timeval syscall_table;
	struct timeval object_table;
	struct timeval memcpy;
	struct timeval encryption;
	struct timeval decryption;
	struct timeval utime;
	struct timeval stime;
	struct timeval total_time;
} sgx_perf;

static FILE* sgx_perf_open_file(){
	
	char *new_name = (char*) malloc(sizeof(file_name) + 4);

	int i;
	for(i = 0; i < 9999; i++){
		sprintf(new_name, "results/%s.%d.txt", ARGV[0], i);
		struct stat buf;
		if(stat(new_name, &buf) != 0)
			return fopen(new_name, "w");
	}

	return NULL;
}


void sgx_perf_init(){
	timerclear(&sgx_perf.main);
	timerclear(&sgx_perf.shim);
	timerclear(&sgx_perf.syscall_table);
	timerclear(&sgx_perf.object_table);
	timerclear(&sgx_perf.memcpy);
	timerclear(&sgx_perf.encryption);
	timerclear(&sgx_perf.decryption);
	timerclear(&sgx_perf.utime);
	timerclear(&sgx_perf.stime);
	timerclear(&sgx_perf.total_time);

	fp = sgx_perf_open_file();
	
	if(getrusage(RUSAGE_SELF, &main_begin))
		perror("(sgx_perf_init)");
}

void sgx_perf_terminate(){

	if(getrusage(RUSAGE_SELF, &main_end))
		perror("(sgx_perf_terminate)");
	
	
	fprintf(fp, "\n=== Summary ===\n");
	
	fprintf(fp, "\n=== Get Resource Usage ===\n");
	struct timeval utime, stime, total_time;
	timersub(&main_end.ru_utime, &main_begin.ru_utime, &utime);
	timersub(&main_end.ru_stime, &main_begin.ru_stime, &stime);
	timeradd(&utime, &stime, &total_time);

	fprintf(fp, "Total time			%ld.%ld s\n", total_time.tv_sec, total_time.tv_usec);
	fprintf(fp, "User time			%ld.%ld s\n", utime.tv_sec, utime.tv_usec);
	fprintf(fp, "System time			%ld.%ld s\n", stime.tv_sec, stime.tv_usec);
	fprintf(fp, "Maximum resident set size: 	%ld KB\n", main_end.ru_maxrss);
	fprintf(fp, "Page reclaims(soft page faults)	%ld\n", main_end.ru_minflt);        
	fprintf(fp, "Page faults (hard page faults)	%ld\n", main_end.ru_majflt);        
	fprintf(fp, "Block input operations 		%ld\n", main_end.ru_inblock);       
	fprintf(fp, "Block output operations 	%ld\n", main_end.ru_oublock);       
	fprintf(fp, "Voluntary context switches 	%ld\n", main_end.ru_nvcsw);         
	fprintf(fp, "Involuntary context switches 	%ld\n", main_end.ru_nivcsw);        

	fprintf(fp, "\n=== Ecall Statistics ===\n");
	fprintf(fp, "\n=== Ocall Statistics ===\n");
	fprintf(fp, "\n=== Syscall Statistics ===\n");
	fprintf(fp, "\n=== Shim Statistics ===\n");

	fclose(fp);

}


