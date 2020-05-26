#include <pthread.h>
#include <syscall.h>
#include <sys/types.h>
#include <string.h>
#include <assert.h>

struct fs_map  fs_table[MAX_THREADS];



static inline struct pthread *__pthread_self()
{
	void *self = NULL;

	__asm__ ("mov %%fs:0,%0" : "=r" (self) );
	
	int i;
	for(i = 0; i< MAX_THREADS; i++){
		if(fs_table[i].tcs == self){
			break;
		}
	}
	assert(i < MAX_THREADS);

	return (struct pthread*)(fs_table[i].fs);
}

#define TP_ADJ(p) (p)

#define MC_PC gregs[REG_RIP]
