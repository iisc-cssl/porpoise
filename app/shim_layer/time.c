#define _GNU_SOURCE
#include <stdio.h>
#include <sched.h>
#include <sys/times.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <stdlib.h>

int main(){

	struct tms main_start, main_end;
	if(times(&main_start)<0){
		perror("(main)");
	}
	nice(-10);

	struct rusage rusage_start, rusage_end;

	if(getrusage(RUSAGE_SELF, &rusage_start))
		perror("(main)");

	
	/*** Application Benchmark Start***/

	int size = 1024 * 1024 ;
	int *ptr = (int*)malloc(size * sizeof(int));
	for ( long i = 0; i < size; i++)
		ptr[i] = -1;

	int fd = open("sample.txt", O_CREAT | O_WRONLY | O_TRUNC);
	for(long i = 0; i <  1024 * 1024 * 10L ; i++)
		write(fd, "a", 1);

	

	/*** Application Benchmark End ***/

	if(getrusage(RUSAGE_SELF, &rusage_end))
		perror("(main)");

	
	if(times(&main_end)<0){
		perror("(main)");
	}

	printf("\n=== Summary ===\n");
	printf("User time:			%.2lf s\n", (main_end.tms_utime - main_start.tms_utime) / 100.0);
	printf("System time time:		%.2lf s\n", (main_end.tms_stime - main_start.tms_stime) / 100.0);

	printf("\n=== Get Resource Usage ===\n");
	struct timeval ru_utime, ru_stime;
	timersub(&rusage_end.ru_utime, &rusage_start.ru_utime, &ru_utime);
	timersub(&rusage_end.ru_stime, &rusage_start.ru_stime, &ru_stime);
	printf("User time:			%ld.%ld s\n", ru_utime.tv_sec, ru_utime.tv_usec);
	printf("System time:			%ld.%ld s\n", ru_stime.tv_sec, ru_stime.tv_usec);
	printf("Maximum resident set size 	%ld KB\n", rusage_end.ru_maxrss);
	printf("Page reclaims(soft page faults)	%ld\n", rusage_end.ru_minflt);        
	printf("Page faults (hard page faults)	%ld\n", rusage_end.ru_majflt);        
	printf("Block input operations 		%ld\n", rusage_end.ru_inblock);       
	printf("Block output operations 	%ld\n", rusage_end.ru_oublock);       
	printf("Voluntary context switches 	%ld\n", rusage_end.ru_nvcsw);         
	printf("Involuntary context switches 	%ld\n", rusage_end.ru_nivcsw);        


	return 0;
}


