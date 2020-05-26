/*
 * enclave/shim_layer/syscall_wrap.cpp
 *
 * Kripa Shanker <kripashanker@iisc.ac.in>
 * Arun Joseph <arunj@iisc.ac.in>
 *
 * __syscall_wrap copies argument of system call from
 * enclave memory to predefined user buffer and copies back
 * result from user buffer to enclave memory.
 */

#include "shim_layer.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <grp.h>
#include <utime.h>
#include <poll.h>
#include <pthread.h>
#include <stdbool.h>
#include <syscall.h>
#include <unistd.h>
#include <sys/syscall.h> 		/*For __NR_*** */
#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/utsname.h>
#include <sys/vfs.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/sysinfo.h>
#include <sys/times.h>
#include <sys/epoll.h>


#define MAX_NO_OBJECTS 1024

struct ptr_map_t {
	void *in;
	void *out;
};

struct ptr_map_t object_table[MAX_NO_OBJECTS];

pthread_spinlock_t syscall_arg_table_lock;
pthread_spinlock_t object_table_lock;


int get_free_syscall_arg_index(){

	pthread_spin_lock(&syscall_arg_table_lock);
	
	int i;
	for(i = 0; i < MAX_NO_OF_THREADS; i++){
		
		if(!syscall_arg_table[i].busy){
			syscall_arg_table[i].busy = 1;
			break;
		}
	}

	pthread_spin_unlock(&syscall_arg_table_lock);


	return i;
}

void *set_out_address(void* in, int size){

	void *out = NULL;
	out = malloc_user(size);
	int allocated = 0;

	pthread_spin_lock(&object_table_lock);
	
	int i;
	for(int i = 0; i < MAX_NO_OBJECTS; i++){
		if(!object_table[i].in){
			object_table[i].in = in;
			object_table[i].out = out;
			allocated = 1;
			break;
		}
	}

	pthread_spin_unlock(&object_table_lock);
	
	if(!allocated){
		free_user(out);
		out = NULL;
	}

	return out;
}

void *get_out_address(void *in){

	void *out = NULL;
	pthread_spin_lock(&object_table_lock);
	
	int i;
	for(i = 0; i < MAX_NO_OBJECTS; i++){
		if(object_table[i].in == in){
			out = object_table[i].out;
			break;
		}
	}

	pthread_spin_unlock(&object_table_lock);
	
	return out;
}

extern "C" int sysctl(struct __sysctl_args *args){
	return (int)syscall(__NR__sysctl, (long)args);
}

long __syscall_wrap(long n, long a1, long a2, long a3, long a4, long a5, long a6)
{

	int syscall_table_index = get_free_syscall_arg_index();

	void *arg0 = syscall_arg_table[syscall_table_index].arg0;
	void *arg1 = syscall_arg_table[syscall_table_index].arg1;
	void *arg2 = syscall_arg_table[syscall_table_index].arg2;
	void *arg3 = syscall_arg_table[syscall_table_index].arg3;
	void *arg4 = syscall_arg_table[syscall_table_index].arg4;
	void *arg5 = syscall_arg_table[syscall_table_index].arg5;
	void *arg6 = syscall_arg_table[syscall_table_index].arg6;
	void *arg7 = syscall_arg_table[syscall_table_index].arg7;
	void *arg8 = syscall_arg_table[syscall_table_index].arg8;
	long *err_no = syscall_arg_table[syscall_table_index].err_no;
	bool null_ptr[7] ;
	null_ptr[1] = null_ptr[2] = null_ptr[3] =null_ptr[4] =null_ptr[5] =null_ptr[6] = 0 ;

	int syscall_wrap_defined = 1;

	send_user(arg0, &n, sizeof(long), 0);

	if(n<0 || n>350){
		int *ptr = NULL;
		*ptr = 0;
	}

	switch(n){


		case __NR_read:		// read(0)
		{
			long fd = a1;
			void *buf = (void*)a2;
			long count = a3;

			send_user(arg1, &fd, sizeof(long), 0);
			send_user(arg3, &count, sizeof(long), 0);
			ocall_syscall(syscall_table_index);

			count = *(long*)arg0;
			if(count > 0)
				recv_user(arg2, buf, count, 0);
			break;
		}

		case __NR_write: 	// write(1)
		{
			long fd = a1;
			void *buf = (void*)a2;
			long count = a3;

			send_user(arg1, &fd, sizeof(long), 0);
			send_user(arg2, buf, a3, 0);
			send_user(arg3, &count, sizeof(long), 0);
			ocall_syscall(syscall_table_index);
			break;
		}

		case __NR_open:		// open(2)
		{
			char *path_name = (char*)a1;
			long flags = a2;
			long mode = a3;

			send_user(arg1, path_name, strlen(path_name)+1, 0);
			send_user(arg2, &flags, sizeof(long), 0);
			send_user(arg3, &mode, sizeof(long), 0);
			ocall_syscall(syscall_table_index);
			break;
		}

		case __NR_close:	// close(3)
		{
			int fd = a1;

			send_user(arg1, &fd, sizeof(fd), 0);

			ocall_syscall(syscall_table_index);
			break;
		}

		case __NR_stat:		// stat(4)
		case __NR_lstat:	// lstat(6)
		{
			void *path_name = (void*)a1;
			struct stat *buf = (struct stat*)a2;

			send_user(arg1, path_name, strlen((const char*)path_name)+1, 0);
			memset(buf, 0, sizeof(*buf));

			ocall_syscall(syscall_table_index);

			if(*(long*)arg0 != -1){
				recv_user(arg2, buf, sizeof(*buf), 0);
			}


			break;
		}

		case __NR_fstat:	// fstat(5)
		{
			long fd = a1;

			send_user(arg1, &a1, sizeof(long), 0);
			ocall_syscall(syscall_table_index);
			recv_user(arg2, (void*)a2, sizeof(struct stat)+1, 0);
			break;
		}

		case __NR_poll:		// poll(7)
		{
			struct pollfd *fds = (struct pollfd*)a1;
			nfds_t nfds = a2;
			int timeout = a3;

			send_user(arg1, fds, sizeof(*fds) * a2, 0);
			send_user(arg2, &nfds, sizeof(nfds), 0);
			send_user(arg3, &timeout, sizeof(timeout), 0);

			ocall_syscall(syscall_table_index);

			recv_user(arg1, fds, sizeof(*fds) * a2, 0);

			break;
		}


		case __NR_lseek:	// lseek(8)
		{
			long fd = a1;
			long offset = a2;
			long whence = a3;

			send_user(arg1, &fd, sizeof(long), 0);
			send_user(arg2, &offset, sizeof(long), 0);
			send_user(arg3, &whence, sizeof(long), 0);

			ocall_syscall(syscall_table_index);

			break;
		}

		case __NR_mmap:		// mmap(9)
		{
			void *addr = (void*)a1;
			size_t length = (size_t)a2;
			long prot = a3;
			long flags = a4;
			long fd = a5;
			off_t offset = a6;

			send_user(arg1, &addr, sizeof(void*), 0);
			send_user(arg2, &length, sizeof(length), 0);
			send_user(arg3, &prot, sizeof(prot), 0);
			send_user(arg4, &flags, sizeof(flags), 0);
			send_user(arg5, &fd, sizeof(fd), 0);
			send_user(arg6, &offset, sizeof(offset), 0);

			ocall_syscall(syscall_table_index);

			break;
		}

		case __NR_mprotect: 	// mprotect(10)
		{
			void *addr = (void*)a1;
			size_t len = (size_t)a2;
			long prot = a3;

			send_user(arg1, &addr, sizeof(void*), 0);
			send_user(arg2, &len, sizeof(len), 0);
			send_user(arg3, &prot, sizeof(prot), 0);

			ocall_syscall(syscall_table_index);

			break;
		}

		case __NR_munmap:	// munmap(11)
		{
			void *addr = (void*)a1;
			size_t length = (size_t)a2;

			send_user(arg1, &addr, sizeof(void*), 0);
			send_user(arg2, &length, sizeof(length), 0);

			ocall_syscall(syscall_table_index);

			break;
		}

		case __NR_brk:		// brk(12)
		{
			void *addr = (void*)a1;
			 send_user(arg1, &addr, sizeof(addr), 0);

			 ocall_syscall(syscall_table_index);

			break;
		}

		case __NR_rt_sigaction:	// rt_sigaction(13)
		{
			long signum = a1;
			struct sigaction *act = (struct sigaction*)a2;
			struct sigaction *act_old = (struct sigaction*)a3;
			size_t sigset_size = (size_t)a4;

			send_user(arg1, &signum, sizeof(signum), 0);
			if(act == NULL){
				*(long*)arg2 = 0;
			} else {
				send_user(arg2, &arg5, sizeof(arg4), 0);
				send_user(arg5, act, sizeof(*act), 0);
				handler_list[signum].sa_sigaction = act->sa_sigaction;
				handler_list[signum].sa_handler = act->sa_handler;
			}

			if(act_old == NULL){
				*(long*)arg3 = 0;
			} else {
				send_user(arg3, &arg6, sizeof(arg5), 0);
			}

			send_user(arg4, &sigset_size, sizeof(sigset_size), 0);

			ocall_syscall(syscall_table_index);

			if(act_old != NULL)
				recv_user(arg6, act_old, 8, 0);
			break;
		}


		case __NR_rt_sigprocmask: 	// rt_sigproc_mask(14)
		{
			long how = a1;
			sigset_t *set = (sigset_t*)a2;
			sigset_t *set_old = (sigset_t*)a3;
			size_t sigsetsize = (size_t)a4;

			send_user(arg1, &how, sizeof(long), 0);
			
			// optimal technique till now to check with set NULL
			if(set){
				*(long*)arg2 = (long)arg5;
				send_user(arg5, set, sizeof(*set), 0);
			} else {
				*(long*)arg2 = 0;
			}

			if(set_old){
				*(long*)arg3 = (long)arg6;
				send_user(arg6, set_old, sizeof(*set_old), 0);
			} else {
				*(long*)arg3 = 0;
			}

			send_user(arg4, &sigsetsize, sizeof(sigsetsize), 0);

			ocall_syscall(syscall_table_index);

			if(set_old)
				recv_user(arg6, set_old, sizeof(*set_old), 0);
			break;
		}

		case __NR_rt_sigreturn:		// rt_sigreturn(15) automatically pushed when a signal came
			break;


		case __NR_ioctl: 	// ioctl(16) 1/200
		{
			int fd = (int)a1;
			unsigned long request = (unsigned long)a2;
			char *argp = (char*)a3;
		
			send_user(arg1, &fd, sizeof(fd), 0);
			send_user(arg2, &request, sizeof(request), 0);
			if(argp)
				send_user(arg3, argp, 32, 0);

			ocall_syscall(syscall_table_index);
			
			if(argp)
				recv_user(arg3, (void*)argp, 32, 0);

			break;
		}

		case __NR_pread64:	// pread64(17)
		case __NR_pwrite64:	// pwrite64(18)
		{
			long fd = a1;
			void *buf = (void*)a2;
			long count = a3;
			long offset = a4;

			send_user(arg1, &fd, sizeof(long), 0);
			send_user(arg3, &count, sizeof(long), 0);

			if(n == __NR_pwrite64)
				send_user(arg2, buf, count, 0);

			send_user(arg4, &offset, sizeof(long), 0);

			ocall_syscall(syscall_table_index);

			count = *(long*)arg0;
			if(n == __NR_pread64)
				recv_user(arg2, buf, count, 0);

			break;
		}

		case __NR_readv:	// readv(19)
		{
			int fd = (int)a1;
			struct iovec *iov_ptr_in = (struct iovec*)a2;
			struct iovec *iov_ptr_out = (struct iovec*)arg2;
			int iovcnt = (int)a3;

			send_user(arg1, &fd, sizeof(fd), 0);

			if(iovcnt >= 1){
				iov_ptr_out[0].iov_len = iov_ptr_in[0].iov_len;
				iov_ptr_out[0].iov_base = arg4;
			}

			if(iovcnt >= 2){
				iov_ptr_out[1].iov_len = iov_ptr_in[1].iov_len;
				iov_ptr_out[1].iov_base = arg5;
			}

			if(iovcnt >= 3){
				iov_ptr_out[2].iov_len = iov_ptr_in[2].iov_len;
				iov_ptr_out[2].iov_base = arg6;
			}

			if(iovcnt >= 4){
				iov_ptr_out[3].iov_len = iov_ptr_in[3].iov_len;
				iov_ptr_out[3].iov_base = arg7;
			}

			if(iovcnt >= 5){
				iov_ptr_out[4].iov_len = iov_ptr_in[4].iov_len;
				iov_ptr_out[4].iov_base = arg8;
			}


			int i = 0;
			for(i = 5; i < iovcnt ; i++){
				int len = iov_ptr_in[i].iov_len;
				void *out = malloc_user(len);
				iov_ptr_out[i].iov_len = len;
				iov_ptr_out[i].iov_base = out;
			}

			send_user(arg3, &iovcnt, sizeof(iovcnt), 0);

			ocall_syscall(syscall_table_index);

			if( iovcnt >= 1)
				recv_user(arg4, iov_ptr_in[0].iov_base, iov_ptr_in[0].iov_len, 0);

			if( iovcnt >= 2)
				recv_user(arg5, iov_ptr_in[1].iov_base, iov_ptr_in[1].iov_len, 0);

			if( iovcnt >= 3)
				recv_user(arg6, iov_ptr_in[2].iov_base, iov_ptr_in[2].iov_len, 0);

			if( iovcnt >= 4)
				recv_user(arg7, iov_ptr_in[3].iov_base, iov_ptr_in[3].iov_len, 0);

			if( iovcnt >= 5)
				recv_user(arg8, iov_ptr_in[4].iov_base, iov_ptr_in[4].iov_len, 0);

			for(i = 5; i < iovcnt ; i++){
				recv_user(iov_ptr_out[i].iov_base, iov_ptr_in[i].iov_base, iov_ptr_in[i].iov_len, 0);
				free_user(iov_ptr_out[i].iov_base);
			}

			break;
		}

		case __NR_writev:	// writev(20)
		{
			int fd = a1;
			struct iovec *iov_in = (struct iovec*)a2;
			struct iovec *iov_out = (struct iovec*)arg2;
			int iovcnt = a3;

			send_user(arg1, &fd, sizeof(fd), 0);
			
			int index = 0;
			if(iovcnt >= 1){
				void *in = iov_in[index].iov_base;
				int len = iov_out[index].iov_len = iov_in[index].iov_len;
				void *out = iov_out[index].iov_base = arg4;
				send_user(out, in, len, 0);
				index++;
			}

			if(iovcnt >= 2){
				void *in = iov_in[index].iov_base;
				int len = iov_out[index].iov_len = iov_in[index].iov_len;
				void *out = iov_out[index].iov_base = arg5;
				send_user(out, in, len, 0);
				index++;
			}

			if(iovcnt >= 3){
				void *in = iov_in[index].iov_base;
				int len = iov_out[index].iov_len = iov_in[index].iov_len;
				void *out = iov_out[index].iov_base = arg6;
				send_user(out, in, len, 0);
				index++;
			}

			if(iovcnt >= 4){
				void *in = iov_in[index].iov_base;
				int len = iov_out[index].iov_len = iov_in[index].iov_len;
				void *out = iov_out[index].iov_base = arg7;
				send_user(out, in, len, 0);
				index++;
			}

			if(iovcnt >= 5){
				void *in = iov_in[index].iov_base;
				int len = iov_out[index].iov_len = iov_in[index].iov_len;
				void *out = iov_out[index].iov_base = arg8;
				send_user(out, in, len, 0);
				index++;
			}


			int i = 0;
			for(i = 5; i < iovcnt ; i++){
				int len = iov_in[i].iov_len;
				void *in = iov_in[i].iov_base;
				void *out = iov_out[i].iov_base = malloc_user(len);
				send_user(out, in, len, 0);
			}


			send_user(arg3, &iovcnt, sizeof(iovcnt), 0);
			ocall_syscall(syscall_table_index);


			for(i = 5; i < iovcnt; i++){
				free_user(iov_out[i].iov_base);
			}

			break;
		}


		case __NR_access:	// access(21)
		{
			char *path_name = (char*)a1;
			long mode = a2;

			send_user(arg1, path_name, strlen(path_name)+1, 0);
			send_user(arg2, &mode, sizeof(long), 0);

			ocall_syscall(syscall_table_index);

			break;
		}

		case __NR_pipe:		//pipe(22)
		{
			int *pipefd = (int*)a1;
			

			ocall_syscall(syscall_table_index);

			recv_user(arg1, pipefd, sizeof(*pipefd) * 2, 0);
			

			break;

		}


		case __NR_select:	// select(23)
		{
			int nfds = (int)a1;
			fd_set *readfds = (fd_set*)a2;
			fd_set *writefds = (fd_set*)a3;
			fd_set *exceptfds = (fd_set*)a4;
			struct timeval *timeout = (struct timeval*)a5;

			if(readfds){
				null_ptr[2] = 0;
				send_user(arg2, readfds, sizeof(*readfds), 0);
			} else {
				null_ptr[2] = 1;
			}

			if(writefds){
				null_ptr[3] = 0;
				send_user(arg3, writefds, sizeof(*writefds), 0);
			} else {
				null_ptr[3] = 1;
			}

			if(exceptfds){
				null_ptr[4] = 0;
				send_user(arg4, exceptfds, sizeof(*exceptfds), 0);
			} else{
				null_ptr[4] = 1;
			}

			if(timeout){
				null_ptr[5] = 0;
				send_user(arg5, timeout, sizeof(*timeout), 0);
			} else {
				null_ptr[5] = 1;
			}	

			send_user(arg7, &null_ptr, sizeof(*null_ptr), 0);

			ocall_syscall(syscall_table_index);

			if(readfds)
				recv_user(arg2, readfds, sizeof(*readfds), 0);

			if(writefds)
				recv_user(arg3, writefds, sizeof(*writefds), 0);
			
			if(exceptfds)
				recv_user(arg4, exceptfds, sizeof(*exceptfds), 0);

			break;
		}


		case __NR_sched_yield: 	// sched_yield(24)
		{
			ocall_syscall(syscall_table_index);
			break;
		}

		case __NR_mremap:	// mremap(25)
		{
			void *old_address = (void*)a1;
			size_t old_size = (size_t)a2;
			size_t new_size = (size_t)a3;
			long flags = a4;
			void *new_address = (void*)a5;

			send_user(arg1, &old_address, sizeof(void*), 0);
			send_user(arg2, &old_size, sizeof(old_size), 0);
			send_user(arg3, &new_size, sizeof(new_size), 0);
			send_user(arg4, &flags, sizeof(flags), 0);
			send_user(arg5, &new_address, sizeof(void*), 0);

			ocall_syscall(syscall_table_index);

			break;
		}

		case __NR_msync:	// msync(26)
		{
			void *addr = (void*)a1;
			size_t length = (size_t)a2;
			long flags = a3;

			send_user(arg1, &addr, sizeof(addr), 0);
			send_user(arg2, &length, sizeof(length), 0);
			send_user(arg3, &flags, sizeof(flags), 0);

			ocall_syscall(syscall_table_index);
			break;
		}

		case __NR_madvise: 	// madvise(28)
		{
			void *addr = (void*)a1;
			size_t length = (size_t)a2;
			long advise = a3;

			send_user(arg1, &addr, sizeof(void*), 0);
			send_user(arg2, &length, sizeof(length), 0);
			send_user(arg3, &advise, sizeof(advise), 0);

			ocall_syscall(syscall_table_index);

			break;
		}

		case __NR_dup:		// dup(32)
		{
			long fd = a1;

			send_user(arg1, &fd, sizeof(fd), 0);

			ocall_syscall(syscall_table_index);

			break;
		}

		case __NR_dup2:		// dup2(33)
		{
			long fd_old = a1;
			long fd_new = a2;

			send_user(arg1, &fd_old, sizeof(long), 0);
			send_user(arg2, &fd_new, sizeof(long), 0);

			ocall_syscall(syscall_table_index);

			break;
		}

		case __NR_nanosleep:	// nanoslepp(35)
		{
			struct timespec *req = (struct timespec*)a1;
			struct timespec *rem = (struct timespec*)a2;

			if(req == NULL)
				*(long*)arg1 = 0;
			else {
				send_user(arg3, req, sizeof(*req), 0);
				send_user(arg1, &arg3, sizeof(void*), 0);
			}

			if(rem == NULL)
				*(long*)arg2 = 0;
			else{
				send_user(arg2, &arg4, sizeof(void*), 0);
			}


			ocall_syscall(syscall_table_index);


			if(rem != NULL)
				recv_user(arg4, rem, sizeof(*rem), 0);


			break;
		}

		case __NR_alarm:	// alarm(37)
		{
			long seconds = a1;

			send_user(arg1, &seconds, sizeof(seconds), 0);

			ocall_syscall(syscall_table_index);
		}

		case __NR_setitimer:	// setitimer(38)
		{
			long which = a1;
			struct itimerval *new_value = (struct itimerval*)a2;
			struct itimerval *old_value = (struct itimerval*)a3;

			send_user(arg1, &which, sizeof(which), 0);
			send_user(arg2, new_value, sizeof(*new_value), 0);
			if(old_value == NULL)
				*(long*)arg3 = 0;
			else
				send_user(arg3, &arg4, sizeof(void*), 0); 	// hack

			ocall_syscall(syscall_table_index);

			if(old_value != NULL)
				recv_user(arg4, old_value, sizeof(*old_value), 0);

			break;
		}

		case __NR_getpid:	// getpid(39)
		{
			ocall_syscall(syscall_table_index);
			break;
		}

		/*---------------------------
		*network system calls 41 -55*
		----------------------------*/
		case __NR_socket:	// socket(41)
			send_user(arg1, &a1, sizeof(long), 0);
			send_user(arg2, &a2, sizeof(long), 0);
			send_user(arg3, &a3, sizeof(long), 0);
			ocall_syscall(syscall_table_index);
			break;

		case __NR_connect:	// connect(42)
			send_user(arg1, &a1, sizeof(long), 0);
			send_user(arg2, (void*)a2, a3, 0);
			send_user(arg3, &a3, sizeof(long), 0);
			ocall_syscall(syscall_table_index);
			break;

		case __NR_accept:	// accept(43)
			send_user(arg1, &a1, sizeof(long), 0);
			send_user(arg2, (void*)a2, *(socklen_t*)a3, 0);
			send_user(arg3, (void*)a3, sizeof(socklen_t), 0);
			ocall_syscall(syscall_table_index);
			break;

		case __NR_sendto:		// sendto(44){
		{
			int sockfd = (int)a1;
			void *buf = (void*)a2;
			size_t len = (size_t)a3;
			int flags = (int)a4;
			struct sockaddr *dest_addr = (struct sockaddr*)a5;
			socklen_t addrlen = (socklen_t)a6;
			

			send_user(arg1, &sockfd, sizeof(sockfd), 0);
			if(buf)
				send_user(arg2, buf, len, 0);
			else 
				null_ptr[2] = 1;
			
			send_user(arg3, &len, sizeof(len), 0);
			send_user(arg4, &flags, sizeof(int), 0);
			
			if(dest_addr)
				send_user(arg5, dest_addr, addrlen, 0);
			else
				null_ptr[5] = 1;

			send_user(arg6, &addrlen, sizeof(addrlen), 0);

			send_user(arg7, &null_ptr, sizeof(*null_ptr), 0);

			ocall_syscall(syscall_table_index);

			break;
		}


		case __NR_recvfrom:	// recvfrom(45)
		{
			int sockfd = (int)a1;
			void *buf = (void*)a2;
			size_t len = (size_t)a3;
			int flags = (int)a4;
			struct sockaddr *dest_addr = (struct sockaddr*)a5;
			socklen_t *addrlen = (socklen_t*)a6;
			

			send_user(arg1, &sockfd, sizeof(sockfd), 0);
			if(buf)
				send_user(arg2, buf, len, 0);
			else 
				null_ptr[2] = 1;
			
			send_user(arg3, &len, sizeof(len), 0);
			send_user(arg4, &flags, sizeof(int), 0);
			
			if(dest_addr)
				send_user(arg5, dest_addr, *addrlen, 0);
			else
				null_ptr[5] = 1;

			if(addrlen){
				send_user(arg6, addrlen, sizeof(*addrlen), 0);
			} else {
				null_ptr[6] = 1;
			}

			send_user(arg7, &null_ptr, sizeof(*null_ptr), 0);

			ocall_syscall(syscall_table_index);

			long bytes_recv = *(long*)arg0;
			if(bytes_recv > 0)
				recv_user(arg2, buf, bytes_recv, 0);

			break;
		}

		case __NR_recvmsg:	// recvmsg(47)
		case __NR_sendmsg:	// sendmsg(46)
		{
			long sockfd = a1;
			struct msghdr *msg_ptr_in = (struct msghdr*)a2;
			struct msghdr *msg_ptr_out = (struct msghdr*)arg2;
			long flags = a3;

			memset(msg_ptr_out, 0, sizeof(struct msghdr));

			send_user(arg1, &sockfd, sizeof(long), 0);

			//sending msg in arg2
			msg_ptr_out->msg_namelen = msg_ptr_in->msg_namelen;
			if(msg_ptr_in->msg_namelen == 0)
			{
				msg_ptr_out->msg_name = 0;
			}
			else
			{
				msg_ptr_out->msg_name = malloc_user(msg_ptr_in->msg_namelen);
				if(n == __NR_sendmsg)
					send_user(msg_ptr_out->msg_name, msg_ptr_in->msg_name, msg_ptr_in->msg_namelen, 0);
			}

			msg_ptr_out->msg_iovlen = msg_ptr_in->msg_iovlen;
			if(msg_ptr_in->msg_iovlen == 0)
			{
				msg_ptr_out->msg_iov = 0;
			}
			else
			{
				msg_ptr_out->msg_iov = (struct iovec*) malloc_user(sizeof(struct iovec) * (msg_ptr_in->msg_iovlen));

				for(long i = 0; i < msg_ptr_in->msg_iovlen; i++)
				{
					struct iovec *iov_elem_in = msg_ptr_in->msg_iov+i;
					struct iovec *iov_elem_out = msg_ptr_out->msg_iov+i;

					iov_elem_out->iov_base = NULL;
					iov_elem_out->iov_len = -1;

					iov_elem_out->iov_len = iov_elem_in->iov_len;
					iov_elem_out->iov_base = malloc_user(iov_elem_in->iov_len);
					if(n == __NR_sendmsg)
						send_user(iov_elem_out->iov_base, iov_elem_in->iov_base, iov_elem_in->iov_len, 0);
				}
			}

			msg_ptr_out->msg_controllen = msg_ptr_in->msg_controllen;
			if(msg_ptr_in->msg_controllen == 0)
			{
				msg_ptr_out->msg_control = 0;
			}
			else
			{
				msg_ptr_out->msg_control = malloc_user(msg_ptr_in->msg_controllen);
				if(n == __NR_sendmsg)
					send_user(msg_ptr_out->msg_control, msg_ptr_in->msg_control, msg_ptr_in->msg_controllen, 0);
			}
			msg_ptr_out->msg_flags = msg_ptr_in->msg_flags;

			send_user(arg3, &flags, sizeof(long), 0);

			ocall_syscall(syscall_table_index);

			if(n == __NR_recvmsg)
			{
				if(msg_ptr_out->msg_namelen == 0);
				else
				{
					recv_user(msg_ptr_out->msg_name, msg_ptr_in->msg_name, msg_ptr_in->msg_namelen, 0);
				}
				if(msg_ptr_out->msg_iovlen == 0);
				else
				{
					for(long i = 0; i < msg_ptr_in->msg_iovlen; i++)
					{
						struct iovec *iov_elem_in = msg_ptr_in->msg_iov+i;
						struct iovec *iov_elem_out = msg_ptr_out->msg_iov+i;
						recv_user(iov_elem_out->iov_base, iov_elem_in->iov_base, iov_elem_in->iov_len, 0);
					}
				}
				if(msg_ptr_out->msg_controllen == 0);
				else
				{
					recv_user(msg_ptr_out->msg_control, msg_ptr_in->msg_control, msg_ptr_in->msg_controllen, 0);
				}
				 msg_ptr_in->msg_flags = msg_ptr_out->msg_flags;
			}
			break;
		}

		case __NR_shutdown:	// shutdown(48)
			send_user(arg1, &a1, sizeof(long), 0);
			send_user(arg2, &a2, sizeof(long), 0);
			ocall_syscall(syscall_table_index);
			break;

		case __NR_bind:		// bind(49)
			send_user(arg1, &a1, sizeof(long), 0);
			send_user(arg2, (void*)a2, a3, 0);
			send_user(arg3, &a3, sizeof(long), 0);
			ocall_syscall(syscall_table_index);
			break;

		case __NR_listen:	// listen(50)
			send_user(arg1, &a1, sizeof(long), 0);
			send_user(arg2, &a2, sizeof(long), 0);
			ocall_syscall(syscall_table_index);
			break;

		case __NR_getsockname:	// getsockname(51) not tested
			send_user(arg1, &a1, sizeof(long), 0);
			send_user(arg2, (void*)a2, *(socklen_t*)a3, 0);
			send_user(arg3, (void*)a3, sizeof(socklen_t), 0);
			ocall_syscall(syscall_table_index);
			break;

		case __NR_getpeername:	// getpeername(52) not tested
			send_user(arg1, &a1, sizeof(long), 0);
			send_user(arg2, (void*)a2, *(socklen_t*)a3, 0);
			send_user(arg3, (void*)a3, sizeof(socklen_t), 0);
			ocall_syscall(syscall_table_index);
			break;

		case __NR_socketpair:	// socketpair(53) not tested
			send_user(arg1, &a1, sizeof(long), 0);
			send_user(arg2, &a2, sizeof(long), 0);
			send_user(arg3, &a3, sizeof(long), 0);
			ocall_syscall(syscall_table_index);
			recv_user(arg4, (void*)a4, 2*sizeof(int), 0);
			break;

		case __NR_setsockopt:	// setsockopt(54)
		{
			int sockfd = (int)a1;
			int level = (int)a2;
			int optname = (int)a3;
			const void *optval = (const void*)a4;
			socklen_t optlen = (socklen_t)a5;

			send_user(arg1, &sockfd, sizeof(sockfd), 0);
			send_user(arg2, &level, sizeof(level), 0);
			send_user(arg3, &optname, sizeof(optname), 0);
			send_user(arg4, optval, optlen, 0);
			send_user(arg5, &optlen, sizeof(socklen_t), 0);
			
			ocall_syscall(syscall_table_index);
			
			break;
		}

		case __NR_getsockopt:	// getsockopt(55) not tested
			send_user(arg1, &a1, sizeof(long), 0);
			send_user(arg2, &a2, sizeof(long), 0);
			send_user(arg3, &a3, sizeof(long), 0);
			send_user(arg4, (void*)a4, *(socklen_t*)a5, 0);
			send_user(arg5, (void*)a5, sizeof(socklen_t), 0);
			ocall_syscall(syscall_table_index);
			break;
		/*-------------------------
		*network system calls ENDS*
		--------------------------*/
		case __NR_clone: 	// clone(56)
		{

			long flags = a1;
			void *child_stack = (void*)a2;
			void *new_tid = (void*)a3;
			void *new_detach = (void*)a4;
			struct clone_arg_t *clone_arg = (struct clone_arg_t*)a5;

			send_user(arg5, &clone_arg, sizeof(clone_arg), 0);

			ocall_syscall(syscall_table_index);

			break;
		}

		
		case __NR_exit:		// exit(60)
		{
			long status = a1;
			send_user(arg1, &status, sizeof(long), 0);
			ocall_syscall(syscall_table_index);
			break;
		}

		case __NR_kill:		// kill(62)
		{
			pid_t pid = (pid_t)a1;
			long sig = a2;

			send_user(arg1, &pid, sizeof(pid), 0);
			send_user(arg2, &sig, sizeof(sig), 0);

			ocall_syscall(syscall_table_index);

			break;
		}

		case __NR_uname: 	// uname(63)
		{
			struct utsname *buf = (struct utsname*)a1;

			ocall_syscall(syscall_table_index);

			recv_user(arg1, buf, sizeof(*buf), 0);

			break;
		}

		case __NR_fcntl:	// fcntl (72)
		{
			int fd = (int)a1;
			int cmd = (int)a2;
			int arg = (int)a3;

			send_user(arg1, &fd, sizeof(fd), 0);
			send_user(arg2, &cmd, sizeof(cmd), 0);
			send_user(arg3, &arg, sizeof(arg), 0);

			ocall_syscall(syscall_table_index);
			break;
		}

		case __NR_fsync:	// fsync(74)
		{
			long fd = a1;

			send_user(arg1, &fd, sizeof(fd), 0);

			ocall_syscall(syscall_table_index);
			break;
		}

		case __NR_truncate: 	// truncate(76)
		{
			char *path_name = (char*)a1;
			mode_t mode = (mode_t)a2;

			send_user(arg1, path_name, sizeof(path_name)+1, 0);
			send_user(arg2, &mode, sizeof(mode), 0);

			ocall_syscall(syscall_table_index);

			break;
		}

		case __NR_ftruncate:	// ftruncate(77)
		{
			long fd = a1;
			mode_t mode = (mode_t)a2;

			send_user(arg1, &fd, sizeof(fd), 0);
			send_user(arg2, &mode, sizeof(mode), 0);

			ocall_syscall(syscall_table_index);

			break;
		}

		case __NR_getdents:	// getdents(78)
		{
			long fd = a1;
			void *dirp = (void*)a2;
			long count = a3;

			send_user(arg1, &fd, sizeof(fd), 0);
			send_user(arg3, &count, sizeof(count), 0);

			ocall_syscall(syscall_table_index);

			recv_user(arg2, dirp, count, 0);

			break;
		}


		case __NR_getcwd:	// getcwd(79)
		{
			char *buf = (char*)a1;
			long size = a2;

			send_user(arg2, &size, sizeof(long), 0);

			ocall_syscall(syscall_table_index);

			if(*(long*)arg0 != 0){
				recv_user(arg1, buf, size, 0);
			}
			break;
		}

		case __NR_chdir: 	// chdir(80)
		{
			char *path_name = (char*)a1;

			send_user(arg1, path_name, strlen(path_name)+1, 0);

			ocall_syscall(syscall_table_index);

			break;
		}


		case __NR_fchdir:	// fchdir(81)
		{
			long fd = a1;

			send_user(arg1, &fd, sizeof(fd), 0);

			ocall_syscall(syscall_table_index);
			break;
		}

		case __NR_rename:	// rename(82)
		{
			char *path_name_old = (char*)a1;
			char *path_name_new = (char*)a2;

			send_user(arg1, path_name_old, strlen(path_name_old)+1, 0);
			send_user(arg2, path_name_new, strlen(path_name_new)+1, 0);

			ocall_syscall(syscall_table_index);
			break;
		}

		case __NR_mkdir:	// mkdir(83)
		{
			char *path_name = (char*)a1;
			mode_t mode = a2;

			send_user(arg1, path_name, strlen(path_name)+1, 0);
			send_user(arg2, &mode, sizeof(long), 0);

			ocall_syscall(syscall_table_index);

			break;
		}

		case __NR_rmdir:	// rmdir(84)
		{
			char *path_name = (char*)a1;

			send_user(arg1, path_name, strlen(path_name)+1, 0);

			ocall_syscall(syscall_table_index);

			break;
		}

		case __NR_creat:	// creat(85)
		{
			char *path_name = (char*)a1;
			long mode = a2;

			send_user(arg1, path_name, strlen(path_name)+1, 0);
			send_user(arg2, &mode, sizeof(long), 0);

			ocall_syscall(syscall_table_index);

			break;
		}

		case __NR_link:		// link(86)
		{
			char *path_name_old = (char*)a1;
			char *path_name_new = (char*)a2;

			send_user(arg1, path_name_old, strlen(path_name_old)+1, 0);
			send_user(arg2, path_name_new, strlen(path_name_new)+1, 0);

			ocall_syscall(syscall_table_index);

			break;
		}

		case __NR_unlink:	// unlink(87)
		{
			char *path_name = (char*)a1;
			send_user(arg1, path_name, strlen(path_name)+1, 0);

			ocall_syscall(syscall_table_index);

			break;
		}

		case __NR_symlink: 	// symlink(88)
		{
			char *path_target = (char*)a1;
			char *path_link = (char*)a2;

			send_user(arg1, path_target, strlen(path_target)+1, 0);
			send_user(arg2, path_link, strlen(path_link)+1, 0);

			ocall_syscall(syscall_table_index);
			break;
		}

		case __NR_readlink:	// readlink(89)
		{
			char *path_name = (char*)a1;
			char *buf = (char*)a2;
			size_t buf_size = a3;

			send_user(arg1, path_name, strlen(path_name)+1, 0);
			send_user(arg3, &buf_size, sizeof(long), 0);

			ocall_syscall(syscall_table_index);

			ssize_t num_of_bytes = *(ssize_t*)arg0;
			if(num_of_bytes > 0)
				recv_user(arg2, buf, num_of_bytes, 0);

			break;
		}

		case __NR_chmod: 	// chmod(90)
		{
			char *path_name = (char*)a1;
			mode_t mode = (mode_t)a2;

			send_user(arg1, path_name, strlen(path_name)+1, 0);
			send_user(arg2, &mode, sizeof(mode_t), 0);

			ocall_syscall(syscall_table_index);

			break;
		}

		case __NR_fchmod:	// fchmod(91)
		{
			long fd = a1;
			mode_t mode = (mode_t)a2;

			send_user(arg1, &fd, sizeof(fd), 0);
			send_user(arg2, &mode, sizeof(mode), 0);

			ocall_syscall(syscall_table_index);

			break;
		}


		case __NR_chown:	// chown(92)
		case __NR_lchown:	// lchown(94)
		{
			char *path_name = (char*)a1;
			uid_t owner = (uid_t)a2;
			gid_t group = (gid_t)a3;

			send_user(arg1, path_name, strlen(path_name)+1, 0);
			send_user(arg2, &owner, sizeof(owner), 0);
			send_user(arg3, &group, sizeof(group), 0);

			ocall_syscall(syscall_table_index);

			break;
		}

		case __NR_fchown:	// fchown(93)
		{
			long fd = a1;
			uid_t owner = (uid_t)a2;
			gid_t group = (gid_t)a3;

			send_user(arg1, &fd, sizeof(fd), 0);
			send_user(arg2, &owner, sizeof(owner), 0);
			send_user(arg3, &group, sizeof(group), 0);

			ocall_syscall(syscall_table_index);

			break;
		}



		case __NR_umask:	// umask(95)
		{
			mode_t mask = (mode_t)a1;

			send_user(arg1, &mask, sizeof(mask), 0);

			ocall_syscall(syscall_table_index);

			break;
		}

		case __NR_gettimeofday:	// gettimeofday(96) not tested
			send_user(arg1, (void*)a1, sizeof(struct timeval), 0);
			send_user(arg2, (void*)a2, sizeof(struct timezone), 0);
			ocall_syscall(syscall_table_index);
			break;

		case __NR_getrlimit: 	// getrlimit(97)
		{
			long resource = a1;
			struct rlimit *rlim = (struct rlimit*)a2;

			send_user(arg1, &resource, sizeof(resource), 0);

			ocall_syscall(syscall_table_index);

			recv_user(arg2, rlim, sizeof(*rlim), 0);

			break;
		}


		case __NR_getrusage:	// getrusage(98)
		{
			long who = a1;
			struct rusage *usage = (struct rusage*)a2;

			send_user(arg1, &who, sizeof(who), 0);

			ocall_syscall(syscall_table_index);

			recv_user(arg2, usage, sizeof(*usage), 0);

			break;
		}

		case __NR_sysinfo:	// sysinfo(99)
		{
			struct sysinfo *info = (struct sysinfo*)a1;

			ocall_syscall(syscall_table_index);

			recv_user(arg1, info, sizeof(*info), 0);

			break;
		}

		case __NR_times:	// times(100)
		{
			struct tms *buf = (struct tms*)a1;

			ocall_syscall(syscall_table_index);

			recv_user(arg1, buf, sizeof(*buf), 0);

			break;
		}

		case __NR_getuid:	// getuid(102)
		{
			ocall_syscall(syscall_table_index);
			break;
		}

		case __NR_getgid:	// getgid(104)
		{
			ocall_syscall(syscall_table_index);
			break;
		}

		case __NR_setuid:	// setuid(105)
		{
			uid_t uid = (uid_t)a1;

			send_user(arg1, &uid, sizeof(long), 0);

			ocall_syscall(syscall_table_index);

			break;
		}

		case __NR_setgid:	// setgid(106) not tested
		{
			gid_t gid = (gid_t)a1;

			send_user(arg1, &gid, sizeof(long), 0);

			ocall_syscall(syscall_table_index);

			break;
		}

		case __NR_geteuid:	// geteuid(107)
		{
			ocall_syscall(syscall_table_index);
			break;
		}

		case __NR_getegid:	// getegid(108)
		{
			ocall_syscall(syscall_table_index);
			break;
		}

		case __NR_setpgid:	// setpgid(109)
		{
			pid_t pid = (pid_t)a1;
			pid_t pgid = (pid_t)a2;

			send_user(arg1, &pid, sizeof(pid), 0);
			send_user(arg2, &pgid, sizeof(pgid), 0);

			ocall_syscall(syscall_table_index);

			break;
		}

		case __NR_getppid:	// getppid(110)
		{
			ocall_syscall(syscall_table_index);

			break;
		}

		case __NR_getpgrp:	// getpgrp(111)
		case __NR_getpgid:	// getpgid(121)
		{
			pid_t pid = (pid_t)a1;

			send_user(arg1, &pid, sizeof(pid_t), 0);

			ocall_syscall(syscall_table_index);
			break;
		}

		case __NR_setsid:	// setsid(112)
		{
			ocall_syscall(syscall_table_index);
			break;
		}

		case __NR_setreuid:	// setreuid(113)
		{
			uid_t ruid = (uid_t)a1;
			uid_t euid = (uid_t)a2;

			send_user(arg1, &ruid, sizeof(ruid), 0);
			send_user(arg2, &euid, sizeof(euid), 0);

			ocall_syscall(syscall_table_index);
			break;
		}

		case __NR_setregid:	// setregid(114)
		{
			gid_t rgid = (gid_t)a1;
			gid_t egid = (gid_t)a2;

			send_user(arg1, &rgid, sizeof(rgid), 0);
			send_user(arg2, &egid, sizeof(egid), 0);

			ocall_syscall(syscall_table_index);

			break;
		}

		case __NR_getgroups: 	// getgroups(115)
		{
			long size = a1;
			gid_t *list = (gid_t*)a2;

			send_user(arg1, &size, sizeof(size), 0);

			ocall_syscall(syscall_table_index);

			if(list != NULL)
				recv_user(arg2, list, size * sizeof(gid_t), 0);
			break;
		}

		case __NR_setgroups:	// setgroups(116)
		{
			long size = a1;
			gid_t *list = (gid_t*)a2;

			send_user(arg1, &size, sizeof(size), 0);
			if(list != NULL)
				send_user(arg2, list, size * sizeof(gid_t), 0);

			ocall_syscall(syscall_table_index);

			break;
		}

		case __NR_setresuid: 	// setresuid(117)
		{
			uid_t ruid = (uid_t)a1;
			uid_t euid = (uid_t)a2;
			uid_t suid = (uid_t)a3;

			send_user(arg1, &ruid, sizeof(uid_t), 0);
			send_user(arg2, &euid, sizeof(uid_t), 0);
			send_user(arg3, &suid, sizeof(uid_t), 0);

			ocall_syscall(syscall_table_index);

			break;
		}


		case __NR_getresuid: 	// getresuid(118)
		{
			uid_t *ruid = (uid_t*)a1;
			uid_t *euid = (uid_t*)a2;
			uid_t *suid = (uid_t*)a3;

			ocall_syscall(syscall_table_index);

			recv_user(arg1, ruid, sizeof(uid_t), 0);
			recv_user(arg2, euid, sizeof(uid_t), 0);
			recv_user(arg3, suid, sizeof(uid_t), 0);

			break;
		}


		case __NR_setresgid: 	// setresgid(119)
		{
			gid_t rgid = (gid_t)a1;
			gid_t egid = (gid_t)a2;
			gid_t sgid = (gid_t)a3;

			send_user(arg1, &rgid, sizeof(gid_t), 0);
			send_user(arg2, &egid, sizeof(gid_t), 0);
			send_user(arg3, &sgid, sizeof(gid_t), 0);

			ocall_syscall(syscall_table_index);

			break;
		}


		case __NR_getresgid: 	// getresgid(120)
		{
			gid_t *rgid = (gid_t*)a1;
			gid_t *egid = (gid_t*)a2;
			gid_t *sgid = (gid_t*)a3;

			ocall_syscall(syscall_table_index);

			recv_user(arg1, rgid, sizeof(gid_t), 0);
			recv_user(arg2, egid, sizeof(gid_t), 0);
			recv_user(arg3, sgid, sizeof(gid_t), 0);

			break;
		}


		case __NR_getsid:	// getsid(124)
		{
			pid_t pid = (pid_t)a1;

			send_user(arg1, &pid, sizeof(pid), 0);

			ocall_syscall(syscall_table_index);

			break;
		}

		case __NR_rt_sigsuspend:  // rt_sigsuspend(130)
		case __NR_rt_sigpending:  // rt_sigpending(127)
		{
			sigset_t *set = (sigset_t *)a1;
			size_t sigsetsize = a2;
			send_user(arg1, set, sizeof(sigset_t), 0);
			send_user(arg2, &sigsetsize, sizeof(size_t), 0);
			ocall_syscall(syscall_table_index);
			break;
		}

		case __NR_rt_sigtimedwait:  // rt_sigpending(128) don't know how to test
		{
			sigset_t *set = (sigset_t *)a1;
			siginfo_t *info = (siginfo_t *)a2;
			timespec *timeout = (timespec *)a3;
			size_t sigsetsize = a4;

			send_user(arg1, set, sizeof(sigset_t), 0);
			send_user(arg2, info, sizeof(siginfo_t), 0);
			send_user(arg3, timeout, sizeof(timespec), 0);
			send_user(arg4, &sigsetsize, sizeof(size_t), 0);

			ocall_syscall(syscall_table_index);
			break;
		}

		case __NR_rt_sigqueueinfo:  // rt_sigqueueinfo (129) not tested
		{
			pid_t tpid = a1;
			int sig = a2;
			siginfo_t *info = (siginfo_t *)a3;

			send_user(arg1, &tpid, sizeof(pid_t), 0);
			send_user(arg2, &sig, sizeof(int), 0);
			send_user(arg3, info, sizeof(siginfo_t), 0);
			ocall_syscall(syscall_table_index);
			break;
		}

		case __NR_sigaltstack: // sigaltstack(131) not completed
		{
			stack_t *signal_stack = (stack_t *)a1;
			stack_t *old_signal_stack = (stack_t *)a2;

			// TODO malloc aacordingly, if needed as of now not implemented
			// typedef struct {
			// 	void  *ss_sp;     /* Base address of stack */
			// 	int    ss_flags;  /* Flags */
			// 	size_t ss_size;   /* Number of bytes in stack */
			// } stack_t;

			if(signal_stack != 0){
				send_user(arg1, signal_stack, sizeof(stack_t), 0);
			}
			ocall_syscall(syscall_table_index);
			if(old_signal_stack != 0){
				recv_user(arg2, old_signal_stack, sizeof(stack_t), 0);
			}
			break;
		}

		
		case __NR_utime:	// utime(132)
		{
			char *file_name = (char*)a1;
			struct utimbuf *times = (struct utimbuf*)a2;

			send_user(arg1, file_name, strlen(file_name)+1, 0);
			if(times!= NULL){
				send_user(arg2, &arg3, sizeof(void*), 0);
				send_user(arg3, times, sizeof(*times), 0);
			} else {
				*(long*)arg2 = 0;
			}

			ocall_syscall(syscall_table_index);
			break;
		}



		case __NR_personality:	// personality(135) not tested
			send_user(arg1, &a1, sizeof(long), 0);
			ocall_syscall(syscall_table_index);
			break;

		case __NR_statfs: 	// statfs(137)
		{
			char *path_name = (char*)a1;
			struct statfs *buf = (struct statfs*)a2;

			send_user(arg1, path_name, strlen(path_name)+1, 0);

			ocall_syscall(syscall_table_index);

			recv_user(arg2, buf, sizeof(*buf), 0);
			break;
		}

		case __NR_fstatfs:	// fstatfs(138)
		{
			long fd = a1;
			struct statfs *buf = (struct statfs*)a2;

			send_user(arg1, &fd, sizeof(fd), 0);

			ocall_syscall(syscall_table_index);

			recv_user(arg2, buf, sizeof(*buf), 0);

			break;
		}

		case __NR_getpriority:	// getpriority(140)
		{
			long which = a1;
			id_t who = (id_t)a2;

			send_user(arg1, &which, sizeof(which), 0);
			send_user(arg2, &who, sizeof(who), 0);

			ocall_syscall(syscall_table_index);

			break;
		}

		case __NR_setpriority:	// setpriority(141)
		{
			long which = a1;
			id_t id = (id_t)a2;
			long priority = a3;

			send_user(arg1, &which, sizeof(which), 0);
			send_user(arg2, &id, sizeof(id), 0);
			send_user(arg3, &priority, sizeof(priority), 0);

			ocall_syscall(syscall_table_index);

			break;
		}

		case __NR_chroot: 	// chroot(161)
		{
			char *path = (char*)a1;

			send_user(arg1, path, strlen(path)+1, 0);

			ocall_syscall(syscall_table_index);

			break;
		}

		case __NR_sync:		// sync(162)
		{
			ocall_syscall(syscall_table_index);

			break;
		}

		case __NR_mount:	// mount(165)
		{
			char *source = (char*)a1;
			char *target = (char*)a2;
			char *file_system_type = (char*)a3;
			unsigned long mount_flags = (unsigned long)a4;
			void *data = (void*)a5;

			send_user(arg1, source, strlen(source) + 1, 0);
			send_user(arg2, target, strlen(target) + 1, 0);
			send_user(arg3, file_system_type, strlen(file_system_type) + 1, 0);
			send_user(arg4, &mount_flags, sizeof(mount_flags), 0);
			if(data == NULL){
				*(long*)arg5 = 0;
			} else {
				send_user(arg5, &arg6, sizeof(arg6), 0);
				send_user(arg6, (char*)data, strlen((char*)data) + 1, 0);
			}

			ocall_syscall(syscall_table_index);

			break;


		}

		case __NR_umount2:	// umount2(166)
		{
			char *target = (char*)a1;
			long flags = a2;

			send_user(arg1, target, strlen(target) + 1, 0);
			send_user(arg2, &flags, sizeof(flags), 0);

			ocall_syscall(syscall_table_index);

			break;
		}


		case __NR_gettid:	// getttid(186)
		{
			ocall_syscall(syscall_table_index);

			break;
		}

		case __NR_futex:	// futex(202)
		{
			int *uaddr = (int*)a1;
			long futex_op = a2;
			long val = a3;
			struct timespec *timeout = (struct timespec *)a4;
			int *uaddr2 = (int*)a5;
			int val3 = a6;

			volatile int *out_uaddr =(volatile int*) get_out_address(uaddr);
			if(out_uaddr == NULL){
				out_uaddr = (volatile int*)set_out_address((void*)uaddr, sizeof(void*));
			} 
		
			send_user((void*)out_uaddr, uaddr, sizeof(*uaddr), 0);
			send_user(arg1, &out_uaddr, sizeof(void*), 0);

			send_user(arg2, &futex_op, sizeof(futex_op), 0);
			send_user(arg3, &val, sizeof(val), 0);
	
			if(timeout == NULL){
				*(long*)arg4 = 0;
			} else {
				send_user(arg4, &arg7, sizeof(void*), 0);
				if(((long)timeout)>>32){
					send_user(arg7, timeout, sizeof(*timeout), 0);
				} else {
					*(long*)arg7 = (long)timeout;
				}
			}	
			
			send_user(arg5, &arg8, sizeof(arg7), 0);
			if(uaddr2)
				send_user(arg8, uaddr2, sizeof(uaddr2), 0);
			else
				*(long*)arg8 = 0;
			
			send_user(arg6, &val3, sizeof(val3), 0);

			ocall_syscall(syscall_table_index);

			// when thread wakes up from sleep, then either internal value can be updated by other thread or outside value is changed by kernel
			/*
			int old = a_swap(out_uaddr, *(int*)uaddr);
			if(old != new_val){
				if(old == 0){
					// kernel has set_tid_address value to 0;
					*(int*)out_uaddr = 0;
					*(int*)uaddr = 0;
				}
				else {
					//its vaue is changed internally

				}
			}
			*/
			break;
		}

		case __NR_sched_getaffinity:	// sched_getaffinity(204)
		{
			pid_t pid = (pid_t)a1;
			size_t cpusetsize = (size_t)a2;
			cpu_set_t *mask = (cpu_set_t *)a3;

			send_user(arg1, &pid, sizeof(pid), 0);
			send_user(arg2, &cpusetsize, sizeof(cpusetsize) , 0);
			send_user(arg3, mask, sizeof(*mask), 0);
			
			ocall_syscall(syscall_table_index);
			
			recv_user(arg3, mask, sizeof(*mask), 0);

			break;


		}

		case __NR_getdents64:		// getdents64(217)
		{
			unsigned int fd = (unsigned int)a1;
			void *dirp = (void*)a2;
			unsigned int count = a3;

			send_user(arg1, &fd, sizeof(fd), 0);
			send_user(arg3, &count, sizeof(count), 0);

			ocall_syscall(syscall_table_index);

			count = *(unsigned int*)arg0;
		
			if(count > 0)
				recv_user(arg2, dirp, count, 0);

			break;

		}

		case __NR_set_tid_address: 	// set_tid_address(218)
		{
			int *tidptr = (int*)a1;

			if(tidptr == NULL)
				*(long*)arg1 = 0;
			else{
				void *out_tidptr = get_out_address(tidptr);
				if(out_tidptr == NULL)
					out_tidptr = set_out_address(tidptr, sizeof(int*));
				send_user(out_tidptr, tidptr, sizeof(int*), 0);
				
				//send_user(arg1, &out_tidptr, sizeof(void*), 0);
				*(long*)out_tidptr = (long)tidptr;
				*(long*)arg1 = (long)out_tidptr;
			}
			
			ocall_syscall(syscall_table_index);

			break;
			
		}

		case __NR_clock_gettime:	// clock_gettime(228)
		{
			clockid_t clk_id = (clockid_t)a1;
			struct timespec *tp = (struct timespec*)a2;

			send_user(arg1, &clk_id, sizeof(clk_id), 0);

			ocall_syscall(syscall_table_index);

			recv_user(arg2, tp, sizeof(*tp), 0);
			break;

		}

		case __NR_clock_getres:	// clock_getres(229)
		{
			clockid_t clk_id = (clockid_t)a1;
			struct timespec *res = (struct timespec*)a2;

			send_user(arg1, &clk_id, sizeof(clk_id), 0);

			ocall_syscall(syscall_table_index);

			if(res != NULL)
				recv_user(arg2, res, sizeof(*res), 0);
			break;
		}


		case __NR_exit_group:	// exit_group(231)
		{
			long status = a1;
			send_user(arg1, &status, sizeof(long), 0);

			ocall_syscall(syscall_table_index);

			break;
		}

		case __NR_epoll_ctl: 	// epoll_ctl(233)
		{
			int epfd = a1;
			int op = (int)a2;
			int fd = (int)a3;
			struct epoll_event *event = (struct epoll_event*)a4;

			send_user(arg1, &epfd, sizeof(epfd), 0);
			send_user(arg2, &op, sizeof(op), 0);
			send_user(arg3, &fd, sizeof(fd), 0);
			if(event){
				send_user(arg5, event, sizeof(struct epoll_event), 0);
				*(long*)arg4 = (long)arg5;
			}else 
				*(long*)arg4 = 0;
				

			ocall_syscall(syscall_table_index);

			break;

		}

		case __NR_utimes:	// utimes(235)
		{
			char *file_name = (char*)a1;
			struct timeval *times = (struct timeval*)a2;

			send_user(arg1, file_name, strlen(file_name)+1, 0);
			if(times!= NULL){
				send_user(arg2, &arg3, sizeof(void*), 0);
				send_user(arg3, times, 2 * sizeof(*times), 0);
			} else {
				*(long*)arg2 = 0;
			}

			ocall_syscall(syscall_table_index);
			
			break;
		}

		case __NR_openat:	// openat(257)
		{
			long dirfd = a1;
			char *path_name = (char*)a2;
			long flags = a3;
			mode_t mode = (mode_t)a3;

			send_user(arg1, &dirfd, sizeof(dirfd), 0);
			send_user(arg2, path_name, strlen(path_name)+1, 0);
			send_user(arg3, &flags, sizeof(flags), 0);
			send_user(arg4, &mode, sizeof(mode), 0);

			ocall_syscall(syscall_table_index);

			break;
		}

		case __NR_fchownat:	// fchownat(260)
		{
			long dirfd = a1;
			char *path_name = (char*)a2;
			uid_t owner = (uid_t)a3;
			gid_t group = (gid_t)a4;
			long flags = a5;

			send_user(arg1, &dirfd, sizeof(dirfd), 0);
			send_user(arg2, path_name, strlen(path_name) + 1, 0);
			send_user(arg3, &owner, sizeof(owner), 0);
			send_user(arg4, &group, sizeof(group), 0);
			send_user(arg5, &flags, sizeof(flags), 0);

			ocall_syscall(syscall_table_index);

			break;
		}

		case __NR_futimesat:	// futimensat(261)
		{
			long fd = a1;
			char *path_name = (char*)a2;
			struct timespec *times = (struct timespec*)a3;

			send_user(arg1, &fd, sizeof(fd), 0);
			send_user(arg2, path_name, strlen(path_name)+1, 0);
			if(times == NULL)
				*(long*)arg3 = 0;
			else{
				send_user(arg3, &arg4, sizeof(arg3), 0);
				send_user(arg4, times, 2 * sizeof(*times), 0);
			}

			ocall_syscall(syscall_table_index);

			break;
		}

		case __NR_newfstatat:	// newfstatat(262)
		{
			long dirfd = a1;
			char *path_name = (char*)a2;
			struct stat *st = (struct stat*)a3;
			long flags = a4;

			send_user(arg1, &dirfd, sizeof(dirfd), 0);
			send_user(arg2, path_name, strlen(path_name)+1, 0);
			send_user(arg4, &flags, sizeof(flags), 0);

			ocall_syscall(syscall_table_index);

			recv_user(arg3, st, sizeof(*st), 0);
			break;
		}

		case __NR_unlinkat:	// unlinkat(263)
		{
			long dirfd = a1;
			char *path_name = (char*)a2;
			long flags = a3;

			send_user(arg1, &dirfd, sizeof(dirfd), 0);
			send_user(arg2, path_name, strlen(path_name) + 1, 0);
			send_user(arg3, &flags, sizeof(flags), 0);

			ocall_syscall(syscall_table_index);
			break;
		}


		case __NR_readlinkat:	// readlinkat(267)
		{
			long dirfd = a1;
			char *path_name = (char*)a2;
			char *buf = (char*)a3;
			size_t buf_size = (size_t)a4;

			send_user(arg1, &dirfd, sizeof(dirfd), 0);
			send_user(arg2, path_name, strlen(path_name) + 1, 0);
			send_user(arg4, &buf_size, sizeof(buf_size), 0);

			ocall_syscall(syscall_table_index);

			buf_size = *(long*)arg0;
			if(buf_size != -1)
				recv_user(arg3, buf, buf_size, 0);
			break;
		}

		case __NR_fchmodat:	// fchmodat(268)
		{
			long dirfd = a1;
			char *path_name = (char*)a2;
			mode_t mode = (mode_t)a3;
			long flags = a4;

			send_user(arg1, &dirfd, sizeof(dirfd), 0);
			send_user(arg2, path_name, strlen(path_name) + 1, 0);
			send_user(arg3, &mode, sizeof(mode), 0);
			send_user(arg4, &flags, sizeof(flags), 0);

			ocall_syscall(syscall_table_index);

			break;
		}

		case __NR_faccessat:	// faccessat(269)
		{
			long dirfd = a1;
			char *path_name = (char*)a2;
			long mode = a3;
			long flags = a4;

			send_user(arg1, &dirfd, sizeof(dirfd), 0);
			send_user(arg2, path_name, strlen(path_name) + 1, 0);
			send_user(arg3, &mode, sizeof(mode), 0);
			send_user(arg4, &flags, sizeof(flags), 0);

			ocall_syscall(syscall_table_index);

			break;
		}

		case __NR_pselect6:	// pselect6(270)
		{
			int nfds = (int)a1;
			fd_set *readfds = (fd_set*)a2;
			fd_set *writefds = (fd_set*)a3;
			fd_set *exceptfds = (fd_set*)a4;
			struct timespec *timeout = (struct timespec*)a5;
			struct pselect6_t { 
				sigset_t *ss; 
				size_t ss_len;
			};

			struct pselect6_t *sigset_wrapper = (struct pselect6_t*)a6;
			

			sigset_t *sigmask = sigset_wrapper->ss;

			if(readfds){
				*(long*)arg2 = (long)arg7;
				send_user(arg7, readfds, sizeof(*readfds), 0);
			} else {
				*(long*)arg2 = 0;
			}

			if(writefds){
				*(long*)arg3 = (long)arg8;
				send_user(arg8, writefds, sizeof(writefds), 0);
			} else {
				*(long*)arg3 = 0;
			}

			void *arg9 = NULL, *arg10 = NULL, *arg11 = NULL;
			if(exceptfds){
				arg9 = malloc_user(sizeof(*exceptfds));
				*(long*)arg4 = (long)arg9;
				send_user(arg9, exceptfds, sizeof(*exceptfds), 0);
			} else{
				*(long*)arg4 = 0;
			}

			if(timeout){
				arg10 = malloc_user(sizeof(*timeout));
				*(long*)arg5 = (long)arg10;
				send_user(arg10, timeout, sizeof(*timeout), 0);
			} else {
				*(long*)arg5 = 0;
			}	

			if(sigmask){
				arg11 = malloc_user( sizeof(*sigmask));
				*(long*)arg5 = (long)arg11;
				send_user(arg11, sigmask, sizeof(*sigmask), 0);
			} else {
				*(long*)arg6 = 0;
			}

			ocall_syscall(syscall_table_index);


			if(arg9)
				free_user(arg9);
			if(arg10)
				free_user(arg10);

			if(arg11)
				free_user(arg11);
			break;
		}
		

		case __NR_utimensat:	// utimensat(280)
		{
			long dirfd = a1;
			char *path_name = (char*)a2;
			struct timespec *times = (struct timespec*)a3;
			long flags = a4;

			send_user(arg1, &dirfd, sizeof(dirfd), 0);
			if(path_name == NULL)
				*(long*)arg2 = 0;
			else{
				send_user(arg2, &arg5, sizeof(arg5), 0);
				send_user(arg5, path_name, strlen(path_name)+1, 0);
			}

			if(times == NULL)
				*(long*)arg3 = 0;
			else{
				send_user(arg3, &arg6, sizeof(arg5), 0);
				send_user(arg6, times, 2 * sizeof(*times), 0);
			}
			send_user(arg4, &flags, sizeof(flags), 0);

			ocall_syscall(syscall_table_index);

			break;

		}

		case __NR_epoll_pwait:	// epoll_pwait(281)
		{
			int epfd = (int)a1;
			struct epoll_event *events = (struct epoll_event*)a2;
			int maxevents = (int)a3;
			int timeout = (int)a4;
			const sigset_t *sigmask = (const sigset_t*)a5;
			size_t sigsetsize = (size_t)a6;
			

			send_user(arg1, &epfd, sizeof(epfd), 0);
			//send_user(arg2, events, sizeof(*events), 0);
			send_user(arg3, &maxevents, sizeof(maxevents), 0);
			send_user(arg4, &timeout, sizeof(timeout), 0);
			if(sigmask)
				send_user(arg5, sigmask, sigsetsize, 0);
			else
				*(long*)arg5 = 0;
			send_user(arg6, &sigsetsize, sizeof(sigsetsize), 0);
	
			ocall_syscall(syscall_table_index);

			int nevents = *(int*)arg0;
			if(nevents > 0)
				recv_user(arg2, events, nevents * sizeof(*events), 0);

			break;
		}

		case __NR_fallocate:	// fallocate(285)
		{
			long fd = a1;
			long mode = a2;
			off_t offset = (off_t)a3;
			off_t len = (off_t)a4;

			send_user(arg1, &fd, sizeof(fd), 0);
			send_user(arg2, &mode, sizeof(mode), 0);
			send_user(arg3, &offset, sizeof(offset), 0);
			send_user(arg4, &len, sizeof(len), 0);

			ocall_syscall(syscall_table_index);

			break;
		}

		case __NR_accept4: 		// __NR_accept4(288)
		{
			int sockfd = (int) a1;
			struct sockaddr *addr = (struct sockaddr*) a2;
			socklen_t *addrlen = (socklen_t*)a3;
			int flags = (int)a4;

			send_user(arg1, &sockfd, sizeof(sockfd), 0);
			if(addrlen)
				send_user(arg3, addrlen, sizeof(*addrlen), 0);
			send_user(arg4, &flags, sizeof(flags), 0);


			ocall_syscall(syscall_table_index);

			if(addr)
				recv_user(arg2, addr, sizeof(*addr), 0);
			if(addrlen)
				recv_user(arg3, addrlen, sizeof(*addrlen), 0);
			
			break;
		}


		case __NR_eventfd2:	// eventfd(290)
		{
			unsigned int initval = (unsigned int)a1;
			int flags = (int)a2;

			send_user(arg1, &initval, sizeof(initval), 0);
			send_user(arg2, &flags, sizeof(flags), 0);

			ocall_syscall(syscall_table_index);

			break;
		}



		case __NR_epoll_create1:	// epoll_create1(291)
		{
			int flags = a1;

			send_user(arg1, &flags, sizeof(flags), 0);
			
			ocall_syscall(syscall_table_index);

			break;

		}

		case __NR_dup3:		// dup3(292)
		{
			long fd_old = a1;
			long fd_new = a2;
			long flags = a3;

			send_user(arg1, &fd_old, sizeof(fd_old), 0);
			send_user(arg2, &fd_new, sizeof(fd_new), 0);
			send_user(arg3, &flags, sizeof(flags), 0);

			ocall_syscall(syscall_table_index);

			break;
		}

		case __NR_pipe2:	// pipe2(293)
		{
			int *pipefd = (int*)a1;
			int flags = (int)a2;

			send_user(arg1, pipefd, 2 * sizeof(*pipefd), 0);
			send_user(arg2, &flags, sizeof(flags), 0);

			ocall_syscall(syscall_table_index);

			recv_user(arg1, pipefd, 2 * sizeof(*pipefd), 0);

			break;
		}

		case __NR_prlimit64:	// prlimit64(302)
		{
			pid_t pid = (pid_t)a1;
			long resource = a2;
			struct rlimit *new_limit = (struct rlimit*)a3;
			struct rlimit *old_limit = (struct rlimit*)a4;

			send_user(arg1, &pid, sizeof(pid), 0);
			send_user(arg2, &resource, sizeof(resource), 0);
			if(new_limit == NULL){
				*(long*)arg3 = 0;
			} else {
				send_user(arg5, new_limit, sizeof(*new_limit), 0);
				send_user(arg3, &arg5, sizeof(void*), 0);
			}

			if(old_limit == NULL)
				*(long*)arg4 = 0;
			else
				send_user(arg4, &arg6, sizeof(void*), 0);

			ocall_syscall(syscall_table_index);

			if(old_limit != NULL){
				recv_user(arg6, old_limit, sizeof(*old_limit), 0);
			}


			break;

		}

		case __NR_syncfs:	// syncfs(306)
		{
			long fd = a1;

			send_user(arg1, &fd, sizeof(fd), 0);

			ocall_syscall(syscall_table_index);

			break;
		}

		case __NR_getrandom: 	// getrandom(318)
		{
			void *buf = (void*)a1;
			size_t buflen = (size_t)a2;
			unsigned int flags = (unsigned int)a3;

			send_user(arg2, &buflen, sizeof(buflen), 0);
			send_user(arg3, &flags, sizeof(flags), 0);


			ocall_syscall(syscall_table_index);

			int count = *(int*)arg0;

			if(count > 0)
				recv_user(arg1, buf, count, 0);

			break;
		}

		default:
		{
			syscall_wrap_defined = 0;
			*(long*)arg0 = -1;
			break;
		}

	}
	
	long ret_val = *(long*)arg0;
	if(ret_val == -1)
		errno = *err_no;
	
	// lock_acquire
	pthread_spin_lock(&syscall_arg_table_lock);
	syscall_arg_table[syscall_table_index].busy = 0;
	pthread_spin_unlock(&syscall_arg_table_lock);
	//lock_release

	if(syscall_wrap_defined == 0)
		printf("syscall_wrap not defined %ld\n", n);
	return ret_val;
}
