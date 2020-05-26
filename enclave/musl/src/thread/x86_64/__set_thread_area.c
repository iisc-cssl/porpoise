#include <sys/types.h>
#include <syscall.h>
#include <pthread.h>
#include <stdint.h>
#include <assert.h>


int __set_thread_area(void *thread_data){

	void *self = NULL;
	__asm__ ("mov %%fs:0,%0" : "=r" (self) );
	
	int i;
	for(i = 0; i<MAX_THREADS; i++){
		if(fs_table[i].tcs == NULL){
			fs_table[i].tcs = self;
			fs_table[i].fs = thread_data;
			break;
		}
	}
	assert(i < MAX_THREADS);

	return 0;
}
