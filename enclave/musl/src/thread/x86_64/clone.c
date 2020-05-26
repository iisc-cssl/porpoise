#include <pthread.h>
#include <syscall.h>
#include <stdlib.h>

int __clone(void (*fn)(void*), void *child_stack, int flags, void *new, void *new_tid, void *new_adj, void *new_detach){

	struct clone_arg_t *clone_arg = (struct clone_arg_t*)malloc(sizeof(*clone_arg));
	clone_arg->arg = new;
	clone_arg->start_routine = fn;

	return syscall(SYS_clone, flags, child_stack, new_tid, new_detach, clone_arg);
}
