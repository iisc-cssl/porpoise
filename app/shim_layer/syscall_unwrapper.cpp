/*
 * app/shim_layer/syscall_unwrapper.cpp
 *
 * Kripa Shanker <kripashanker@iisc.ac.in>
 * Arun Joseph <arunj@iisc.ac.in>
 *
 * This will unwrap the arguments of system call and will call the system call
 *
 */

#include <stdio.h>
#include <sys/syscall.h>
#include <unistd.h>
#include "shim_layer.h"
#include <sys/utsname.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/vfs.h>
#include <signal.h>
#include <errno.h>
#include <grp.h>
#include <sys/mman.h>
#include <utime.h>
#include <sys/times.h>
#include <sys/socket.h>
#include <pthread.h>
#include <poll.h>

#include <sys/types.h>
#include <sys/socket.h>

void ocall_print_string(char *str)
{
	puts(str);
}

void ocall_syscall(int syscall_table_index)
{

	#ifdef PERFORMANCE_ANALYSIS
	unsigned int t3=0,t4=0;
	struct timespec end_rt;
	performance->no_of_syscalls += 1;
	#endif


	long *arg0 = syscall_arg_table[syscall_table_index].arg0;
	long *arg1 = syscall_arg_table[syscall_table_index].arg1;
	long *arg2 = syscall_arg_table[syscall_table_index].arg2;
	long *arg3 = syscall_arg_table[syscall_table_index].arg3;
	long *arg4 = syscall_arg_table[syscall_table_index].arg4;
	long *arg5 = syscall_arg_table[syscall_table_index].arg5;
	long *arg6 = syscall_arg_table[syscall_table_index].arg6;
	long *arg7 = syscall_arg_table[syscall_table_index].arg7;
	long *arg8 = syscall_arg_table[syscall_table_index].arg8;
	long *err_no = syscall_arg_table[syscall_table_index].err_no;

	int n = *(int*)arg0;
	long a1;
	long a2;
	long a3;
	long a4;
	long a5;
	long a6;

	bool *null_ptr = (bool*)arg7;

	if(n > 350 || n <0){
		printf("syscall invalid %d\n", n);
		int *ptr =NULL;
		*ptr = 0;
	}

	switch(n){
		case __NR_read:		// read(0)
		{
			int fd = *arg1;
			void *buf = (void*)arg2;
			size_t count = *(size_t*)arg3;
			*arg0 = syscall(SYS_read, fd, buf, count);

			if(*arg0 == -1){
				printf("read(%d, %p, %lu) = %ld errno %d\n", fd, buf, count, *arg0, errno);
			}

			break;
		}

		case __NR_write:	// write(1)
			a1 = *arg1;
			a2 = (long)arg2;
			a3 = *arg3;
			*arg0 = syscall(n, a1, a2, a3);
			break;

		case __NR_open:		// open(2)
			a1 = (long)arg1;
			a2 = *arg2;
			a3 = *arg3;
			*arg0 = syscall(n, a1, a2, a3);
			break;

		case __NR_close:	// close(3)
		{
			int fd = *(int*)arg1;
			
			*arg0 = syscall(n, fd);
			
			break;
		}

		case __NR_stat:		// stat(4)
		case __NR_lstat:	// lstat(5)
			a1 = (long)arg1;
			a2 = (long)arg2;
			*arg0 = syscall(n, a1, a2);
			break;

		case __NR_fstat:	// fstat(6)
			a1 = *arg1;
			a2 = (long)arg2;
			*arg0 = syscall(n, a1, a2);
			break;


		case __NR_poll:		// poll(7)
		{
			struct pollfd *fds = (struct pollfd*)arg1;
			nfds_t nfds = *(nfds_t*)arg2;
			int timeout = *(int*)arg3;

			*arg0 = syscall(n, fds, nfds, timeout);

			break;
		}

		case __NR_lseek:	// lseek(8)
			a1 = *arg1;
			a2 = *arg2;
			a3 = *arg3;
			*arg0 = syscall(n, a1, a2);
			break;

		case __NR_mmap: 	// mmap(9)
		{
			void *addr = (void*)*(long*)arg1;
			size_t length = *(size_t*)arg2;
			int prot = *(int*)arg3;
			int flags = *(int*)arg4;
			int fd = *(int*)arg5;
			off_t offset = *(off_t*)arg6;

			*arg0 = syscall(n, addr, length, prot, flags, fd, offset);

			if((void*)*(long*)arg0 == MAP_FAILED)
				printf("mmap error %d\n", errno);

			break;
		}

		case __NR_mprotect: 	// mprotect(10)
		{
			void *addr = (void*)*(long*)arg1;
			size_t len = *(size_t*)arg2;
			long prot = *arg3;

			*arg0 = syscall(n, addr, len, prot);

			break;
		}

		case __NR_munmap: 	// munmap(11)
		{
			void *addr = (void*)*(long*)arg1;
			size_t length = *(size_t*)arg2;

			*arg0 = syscall(n, addr, length);

			break;
		}

		case __NR_brk:		// brk(12)
		{
			void *addr = (void*)*(long*)arg1;

			*arg0 = syscall(n, addr);

			break;
		}


		case __NR_rt_sigaction:	// rt_sigaction(13){
		{
			long signum = *arg1;
			struct sigaction *act = (struct sigaction*)*(long*)arg2;
			struct sigaction *act_old = (struct sigaction*)*(long*)arg3;
			if(act != NULL){
				act->sa_sigaction = outside_sig_handler;
				act->sa_flags = SA_SIGINFO | SA_NODEFER | SA_RESTART;
			}

			*arg0 = sigaction(signum, NULL, act_old);

			break;
		}


		case __NR_rt_sigprocmask: 	// rt_sigproc_mask(14)
		{
			int how = *(int*)arg1;
			sigset_t *set = (sigset_t*)*(long*)arg2;
			sigset_t *set_old = (sigset_t*)*(long*)arg3;
			size_t sigsetsize = *(size_t*)arg4;

			*arg0 = syscall(n, how, set, set_old, sigsetsize);	// 8 is hardcoded in musl
			break;
		}

		case __NR_rt_sigreturn:		// rt_sigreturn(15) automatically pushed when a signal came
			break;

		case __NR_ioctl:	// ioctl(16)
		{
			int fd = *(int*)arg1;
			unsigned long request = *(unsigned long*)arg2;
		
			*arg0 = syscall(n, fd, request, arg3);
			
			break;
		}

		case __NR_pread64:	// pread64(17)
		case __NR_pwrite64:	// pwrite64(18)
		{
			long fd = *arg1;
			long buf = (long)arg2;
			long count = *arg3;
			long offset = *arg4;
			*arg0 = syscall(n, fd, buf, count, offset);
			break;
		}

		case __NR_readv:	// readv(19)
		case __NR_writev:	// writev(20)
			a1 = *arg1;
			a2 = (long)arg2;
			a3 = *arg3;
			*arg0 = syscall(n, a1, a2, a3);
			break;


		case __NR_access:	// access(21)
			a1 = (long)arg1;
			a2 = *arg2;
			*arg0 = syscall(n, a1, a2);
			break;

		case __NR_pipe:		//pipe(22)
		{
			int *pipefd = (int*)arg1;

			*arg0 = syscall(n, pipefd);
			break;


		}



		case __NR_select:	// select6(23)
		{
			int nfds = *(int*)arg1;
			fd_set *readfds = (fd_set*)arg2;
			fd_set *writefds = (fd_set*)arg3;
			fd_set *exceptfds = (fd_set*)arg4;
			struct timeval *timeout = (struct timeval*)arg5;

			if(null_ptr[2])
				readfds = NULL;

			if(null_ptr[3])
				writefds = NULL;

			if(null_ptr[4])
				exceptfds = NULL;

			if(null_ptr[5])
				timeout = NULL;


			*arg0 = syscall(SYS_select, nfds, readfds, writefds, exceptfds, timeout);

			break;
		}


		case __NR_sched_yield: 	// sched_yield(24)
		{
			*arg0 = syscall(n);
			
			break;
		}

		case __NR_mremap:	// mremap(25)
		{
			void *old_address = (void*)*(long*)arg1;
			size_t old_size = *(size_t*)arg2;
			size_t new_size = *(size_t*)arg3;
			long flags = *arg4;
			void *new_address = (void*)*(long*)arg5;

			*arg0 = syscall(n, old_address, old_size, new_size, flags, new_address);

			break;
		}


		case __NR_msync:	// msync(26)
		{
			void *addr = (void*)*(long*)arg1;
			size_t length = *(size_t*)arg2;
			long flags = *arg3;

			*arg0 = syscall(n, addr, length, flags);

			break;

		}


		case __NR_madvise: 	// madvise(28)
		{
			void *addr = (void*)*(long*)arg1;
			size_t length = *(size_t*)arg2;
			int advise = *(int*)arg3;

			*arg0 = syscall(n, addr, length, advise);

			break;
		}

		case __NR_dup:		// dup(32)
		{
			long fd = *arg1;

			*arg0 = syscall(n, fd);

			break;
		}

		case __NR_dup2: 	// dup2(33)
		{
			long fd_old = *arg1;
			long fd_new = *arg2;

			*arg0 = syscall(n, fd_old, fd_new);

			break;
		}

		case __NR_nanosleep:	// nanosleep(35)
		{

			struct timespec *req = (struct timespec*)*(long*)arg1;
			struct timespec *rem = (struct timespec*)*(long*)arg2;

			*arg0 = syscall(n, req, rem);

			break;
		}

		case __NR_getitimer: 	// gettimer(36)
			a1 = *arg1;
			a2 = (long)arg2;
			*arg0 = syscall(n, a1, a2);
			break;

		case __NR_alarm:	// alarm(37)
		{
			long seconds = *arg1;

			*arg0 = syscall(n, seconds);

			break;
		}


		case __NR_setitimer:	// setitimer(38)
		{
			long which = *arg1;
			struct itimerval *new_value = (struct itimerval*)arg2;
			struct itimerval *old_value = (struct itimerval*)*(long*)arg3;

			*arg0 = syscall(n, which, new_value, old_value);

			break;
		}

		case __NR_getpid:	// getpid(39)
		{
			*arg0 = syscall(n);
			break;
		}

		/*---------------------------
		*network system calls 41 -55*
		----------------------------*/
		case __NR_socket:	// socket(41)
		{
			long domain = *arg1;
			long type = *arg2;
			long protocol = *arg3;

			*arg0 = syscall(n, domain, type, protocol);
			break;
		}

		case __NR_connect:	// connect(42)
			a1 = *arg1;
			a2 = (long)arg2;
			a3 = *arg3;
			*arg0 = syscall(n, a1, a2, a3);
			break;

		case __NR_accept:	// accept(43)
			a1 = *arg1;
			a2 = (long)arg2;
			a3 = (long)arg3;
			*arg0 = syscall(n, a1, a2, a3);
			break;

		case __NR_sendto:	// sendto(44)
		{
			int sockfd = *(int*)arg1;
			void *buf = (void*)arg2;
			size_t len = *(size_t*)arg3;
			int flags = *(int*)arg4;
			const struct sockaddr *dest_addr = (const struct sockaddr*)arg5;
			socklen_t addrlen = *(socklen_t*)arg6;


			if(null_ptr[2])
				buf = NULL;
			if(null_ptr[5])
				dest_addr = NULL;

			*arg0 = syscall(n, sockfd, buf, len, flags, dest_addr, addrlen);
			break;
		}


		case __NR_recvfrom:	// recvfrom(45)
		{
			int sockfd = *(int*)arg1;
			void *buf = (void*)arg2;
			size_t len = *(size_t*)arg3;
			int flags = *(int*)arg4;
			const struct sockaddr *dest_addr = (const struct sockaddr*)arg5;
			socklen_t *addrlen = (socklen_t*)arg6;


			if(null_ptr[2])
				buf = NULL;
			if(null_ptr[5])
				dest_addr = NULL;
			if(null_ptr[6])
				addrlen = NULL;

			*arg0 = syscall(n, sockfd, buf, len, flags, dest_addr, addrlen);
			break;
		}

		case __NR_sendmsg:	// sendmsg(46)
		case __NR_recvmsg:	// recvmsg(47)
		{
			long sockfd = *arg1;
			long msg = (long)arg2;
			long flags = *arg3;
			*arg0 = syscall(n, sockfd, msg, flags);
			break;
		}

		case __NR_shutdown:	// shutdown(48)
		{
			long sockfd = *arg1;
			long how = *arg2;

			*arg0 = syscall(n, sockfd, how);
			break;
		}

		case __NR_bind:		// bind(49)
			a1 = *arg1;
			a2 = (long)arg2;
			a3 = *arg3;
			*arg0 = syscall(n, a1, a2, a3);
			break;

		case __NR_listen:	// listen(50)
			a1 = *arg1;
			a2 = *arg2;
			*arg0 = syscall(n, a1, a2);
			break;

		case __NR_getsockname:	// getsockname(51) not tested
			a1 = *arg1;
			a2 = (long)arg2;
			a3 = (long)arg3;
			*arg0 = syscall(n, a1, a2, a3);
			break;

		case __NR_getpeername:	// getpeername(52) not tested
			a1 = *arg1;
			a2 = (long)arg2;
			a3 = (long)arg3;
			*arg0 = syscall(n, a1, a2, a3);
			break;

		case __NR_socketpair:	// sockpair(53) not tested
			a1 = *arg1;
			a2 = *arg2;
			a3 = *arg3;
			a4 = (long)arg4;
			*arg0 = syscall(n, a1, a2, a3, a4);
			break;

		case __NR_setsockopt:	// setsockopt(54)
		{
			int sockfd  = *(int*)arg1;
			int level = *(int*)arg2;
			int optname = *(int*)arg3;
			const void *optval = (const void*)arg4;
			socklen_t optlen = *(socklen_t*)arg5;

			*arg0 = syscall(n, sockfd, level, optname, optval, optlen);
			
			break;
		}

		case __NR_getsockopt:	// getsockopt(55) not tested
			a1 = *arg1;
			a2 = *arg2;
			a3 = *arg3;
			a4 = (long)arg4;
			a5 = (long)arg5;
			*arg0 = syscall(n, a1, a2, a3, a4, a5);
			break;
		/*-------------------------
		*network system calls ENDS*
		-------------------------*/

		case __NR_clone: 	// clone(56)
		{

			void *clone_arg = malloc(sizeof(clone_arg));
			clone_arg = (void*)*arg5;

			pthread_t tid;
			int status;
			if(status = pthread_create(&tid, NULL, out_start_routine, (void*)clone_arg) ==0){
				*arg0 = 1;
			}
			else{
				*arg0 = -1;
				printf("pthread_create failed %d\n", status );
			}

			break;
		}

		case __NR_exit:		// exit(60)
		{
			a1 = *arg1;

			*arg0 = syscall(n, a1);

			break;
		}

		case __NR_kill:		// kill(62)
		{
			pid_t pid = *(pid_t*)arg1;
			long sig = *arg2;

			*arg0 = syscall(n, pid, sig);

			break;
		}

		case __NR_uname:	// uname(63)
		{
			struct utsname *buf = (struct utsname*)arg1;

			*arg0 = syscall(n, buf);

			break;
		}
		case __NR_fcntl:	// fcntl (72)
		{
			int fd = *(int*)arg1;
			int cmd = *(int*)arg2;
			int arg = *(int*)arg3;

			*arg0 = syscall(n, fd, cmd, arg);

			break;
		}


		case __NR_fsync:	// fsync(74)
		{
			long fd = *arg1;

			*arg0 = syscall(n, fd);

			break;
		}
		case __NR_truncate: 	// truncate(76)
		{
			char *path_name = (char*)arg1;
			mode_t mode = *(mode_t*)arg2;

			*arg0 = syscall(n, path_name, mode);

			break;
		}

		case __NR_ftruncate:	// ftruncate(77)
		{
			long fd = *arg1;
			mode_t mode = *(mode_t*)arg2;

			*arg0 = syscall(n, fd, mode);

			break;
		}

		case __NR_getdents:	// getdents(78)
		{
			long fd = *arg1;
			void *dirp = arg2;
			long count = *arg3;

			*arg0 = syscall(n, fd, dirp, count);

			break;
		}


		case __NR_getcwd:	// getcwd(79)
			a1 = (long)arg1;
			a2 = *arg2;
			*arg0 = syscall(n, a1, a2);
			break;

		case __NR_chdir: 	// chdir(80)
		{
			char *path_name = (char*)arg1;

			*arg0 = syscall(n, path_name);
			break;
		}


		case __NR_fchdir:	// fchdir(81)
		{
			long fd = *arg1;

			*arg0 = syscall(n, fd);
			break;
		}

		case __NR_rename:	// rename(82)
		{
			char *path_name_old = (char*)arg1;
			char *path_name_new = (char*)arg2;

			*arg0 = syscall(n, path_name_old, path_name_new);
			break;
		}


		case __NR_mkdir:	// mkdir(83)
		{
			char *path_name = (char*) arg1;
			mode_t mode = *(mode_t*)arg2;

			*arg0 = syscall(n, path_name, mode);
			break;
		}

		case __NR_rmdir:	// rmdir(84)
		{
			char *path_name = (char*) arg1;

			*arg0 = syscall(n, path_name);
			break;
		}


		case __NR_creat:	// creat(85)
		{
			long path_name = (long)arg1;
			long mode = *arg2;
			*arg0 = syscall(n, path_name, mode);
			break;
		}

		case __NR_link:		// link(86)
		{
			char *path_name_old = (char*)arg1;
			char *path_name_new = (char*)arg2;
			*arg0 = syscall(n, path_name_old, path_name_new);
			break;
		}

		case __NR_unlink:	// unlink(87)
		{
			char *path_name = (char*)arg1;
			*arg0 = syscall(n, path_name);
			break;
		}

		case __NR_symlink:	// symlink(88)
		{
			char *path_target = (char*)arg1;
			char *path_link = (char*)arg2;

			*arg0 = syscall(n, path_target, path_link);
			break;
		}

		case __NR_readlink: 	// readlink(89)
		{
			char *path_name = (char*)arg1;
			char *buf = (char*)arg2;
			size_t buf_size = *arg3;

			*arg0 = syscall(n, path_name, buf, buf_size);
			break;
		}

		case __NR_chmod: 	// chmod(90)
		{
			char *path_name = (char*)arg1;
			mode_t mode = *(mode_t*)arg2;

			*arg0 = syscall(n, path_name, mode);
			break;
		}

		case __NR_fchmod:	// fchmod(91)
		{
			long fd = *arg1;
			mode_t mode = *(mode_t*)arg2;

			*arg0 = syscall(n, fd, mode);

			break;

		}
		case __NR_chown:	// chown(92)
		case __NR_lchown:	// lchown(94)
		{
			char *path_name = (char*)arg1;
			uid_t owner = *(uid_t*)arg2;
			gid_t group = *(gid_t*)arg3;

			*arg0 = syscall(n, path_name, owner, group);
			break;
		}

		case __NR_fchown:	// fchown(93)
		{
			long fd = *arg1;
			uid_t owner = *(uid_t*)arg2;
			gid_t group = *(gid_t*)arg3;

			*arg0 = syscall(n, fd, owner, group);
			break;
		}



		case __NR_umask:	// umask(95)
		{
			mode_t mask = *(mode_t*)arg1;

			*arg0 = syscall(n, mask);

			break;
		}

		case __NR_gettimeofday: // gettimeofday(96)
			a1 = (long)arg1;
			a2 = (long)arg2;
			*arg0 = syscall(n, a1, a2);
			break;

		case __NR_getrlimit:	// getrlimit(97)
		{
			long resource = *arg1;
			struct rlimit *rlim = (struct rlimit *)arg2;

			*arg0 = syscall(n, resource, rlim);

			break;
		}

		case __NR_getrusage:	// getrusage(98)
		{
			long who = *arg1;
			struct rusage *usage = (struct rusage*)arg2;

			*arg0 = syscall(n, who, usage);
			break;
		}



		case __NR_sysinfo:	// sysinfo(99)
		{
			struct sysinfo *info = (struct sysinfo*)arg1;

			*arg0 = syscall(n, info);

			break;
		}

		case __NR_times:	// times(100)
		{
			struct tms *buf = (struct tms*)arg1;

			*arg0 = syscall(n, buf);

			break;
		}

		case __NR_getuid:	// getuid(102)
		{
			*arg0 = syscall(n);
			break;
		}

		case __NR_getgid:	// getgid(104)
		{
			gid_t gid = *(gid_t*)arg1;

			*arg0 = syscall(n, gid);
			break;
		}

		case __NR_setuid:	// setuid(105)
		{
			uid_t uid = *arg1;

			*arg0 = syscall(n, uid);
			break;
		}

		case __NR_setgid:	// setgid(106)
			a1 = *arg1;
			*arg0 = syscall(n, a1);
			break;

		case __NR_geteuid:	// seteuid(107)
			*arg0 = syscall(n, a1);
			break;

		case __NR_getegid:	// getegid(108)
			*arg0 = syscall(n, a1);
			break;

		case __NR_setpgid:	// setpgid(109)
			a1 = *arg1;
			a2 = *arg2;
			*arg0 = syscall(n, a1, a2);
			break;

		case __NR_getppid:	// getppid(110)
			*arg0 = syscall(n, a1);
			break;

		case __NR_getpgrp:	// getpgrp(111)
		case __NR_getpgid:	// getpgid(121)
		{
			pid_t pid = *(pid_t*)arg1;
			*arg0 = syscall(n, pid);
			break;
		}

		case __NR_setsid:	// setsid(112)
		{
			*arg0 = syscall(n);
			break;
		}

		case __NR_setreuid: 	// setreuid(113)
		{
			uid_t ruid = *(uid_t*)arg1;
			uid_t euid = *(uid_t*)arg2;

			*arg0 = syscall(n, ruid, euid);

			break;
		}

		case __NR_setregid: 	// setregid(114)
		{
			gid_t rgid = *(gid_t*)arg1;
			gid_t egid = *(gid_t*)arg2;

			*arg0 = syscall(n, rgid, egid);

			break;
		}

		case __NR_getgroups: 	// getgroups(115)
		case __NR_setgroups: 	// setgroups(116)
		{
			long size = *arg1;
			gid_t *list = (gid_t*)arg2;

			*arg0 = syscall(n, size, list);

			break;
		}


		case __NR_setresuid: 	// setresuid(117)
		{
			uid_t ruid = *(uid_t*)arg1;
			uid_t euid = *(uid_t*)arg2;
			uid_t suid = *(uid_t*)arg3;

			*arg0 = syscall(n, ruid, euid, suid);

			break;
		}

		case __NR_getresuid: // getresuid(118)
		{
			uid_t *ruid = (uid_t*)arg1;
			uid_t *euid = (uid_t*)arg2;
			uid_t *suid = (uid_t*)arg3;

			*arg0 = syscall(n, ruid, euid, suid);

			break;

		}


		case __NR_setresgid: 	// setresgid(119)
		{
			gid_t rgid = *(gid_t*)arg1;
			gid_t egid = *(gid_t*)arg2;
			gid_t sgid = *(gid_t*)arg3;

			*arg0 = syscall(n, rgid, egid, sgid);

			break;
		}

		case __NR_getresgid: // getresgid(120)
		{
			gid_t *rgid = (gid_t*)arg1;
			gid_t *egid = (gid_t*)arg2;
			gid_t *sgid = (gid_t*)arg3;

			*arg0 = syscall(n, rgid, egid, sgid);

			break;

		}

		case __NR_getsid:	// getsid(124)
		{
			pid_t pid = *(pid_t*)arg1;

			*arg0 = syscall(n, pid);

			break;
		}

		case __NR_rt_sigsuspend:  // rt_sigsuspend(130)
		case __NR_rt_sigpending:  // rt_sigpending(127)
		{
			sigset_t *set = (sigset_t *)arg1;
			size_t sigsetsize = *arg2;
			*arg0 = syscall(n, set, sigsetsize);
			break;
		}

		case __NR_rt_sigtimedwait:  // rt_sigtimedwait(128) not tested
		{
			sigset_t *set = (sigset_t *)arg1;
			siginfo_t *info = (siginfo_t *)arg2;
			timespec *timeout = (timespec *)arg3;
			size_t sigsetsize = *arg4;
			*arg0 = syscall(n, set, info, timeout, sigsetsize);
			break;
		}

		case __NR_rt_sigqueueinfo:  // rt_sigqueueinfo (129) not tested
		{
			pid_t tpid = *arg1;
			int sig = *arg2;
			siginfo_t *info = (siginfo_t *)arg3;

			*arg0 = syscall(n, tpid, sig, info);
			break;
		}

		case __NR_sigaltstack: // sigaltstack(131) not completed
		{
			stack_t *signal_stack = (stack_t *)arg1;
			stack_t *old_signal_stack = (stack_t *)arg2;

			*arg0 = syscall(n, signal_stack, old_signal_stack);
			break;
		}

		case __NR_utime:	// utime(132)
		{
			char *file_name = (char*)arg1;
			struct utimbuf *times = (struct utimbuf*)*(long*)arg2;

			*arg0 = syscall(n, file_name, times);


			break;
		}
		case __NR_personality:	// personality(135)
			a1 = *arg1;
			*arg0 = syscall(n, a1);
			break;

		case __NR_statfs:	// statfs(137)
		{
			char *path_name = (char*)arg1;
			struct statfs *buf = (struct statfs*)arg2;

			*arg0 = syscall(n, path_name, buf);
			break;
		}

		case __NR_fstatfs:	// fstatfs(138)
		{
			long fd = *arg1;
			struct statfs *buf = (struct statfs*)arg2;

			*arg0 = syscall(n, fd, buf);
			break;

		}

		case __NR_getpriority:	// getpriority(140)
		{
			long which = *arg1;
			id_t who = *(id_t*)arg2;

			*arg0 = syscall(n, which, who);

			break;
		}

		case __NR_setpriority:	// setpriority(141)
		{
			long which = *arg1;
			id_t id = *(id_t*)arg2;
			long priority = *arg3;

			*arg0 = syscall(n, which, id, priority);

			break;
		}

		case __NR_chroot: 	// chroot(161)
		{
			char *path = (char*)arg1;

			*arg0 = syscall(n, path);

			break;
		}

		case __NR_sync:		// sync(162)
		{
			syscall(n);
		}

		case __NR_mount:	// mount(165)
		{
			char *source = (char*)arg1;
			char *target = (char*)arg2;
			char *file_system_type = (char*)arg3;
			unsigned long mount_flags = *(unsigned long*)arg4;
			void *data = (void*)*(long*)arg5;

			*arg0 = syscall(n, source, target, file_system_type, mount_flags, data);

			break;
		}

		case __NR_umount2:	// umount2(166)
		{
			char *target = (char*)arg1;
			long flags = *arg2;

			*arg0 = syscall(n, target, flags);

			break;
		}


		case __NR_gettid:	// gettid(186)
			*arg0 = syscall(n);
			break;

		case __NR_futex:	// futex(202)
		{
			int *uaddr = (int*)*(long*)arg1;
			long futex_op = *arg2;
			long val = *arg3;
			struct timespec *timeout = (struct timespec *)*(long*)arg4;
			int *uaddr2 = (int*)arg5;
			long val3 = *arg5;

			*arg0 = syscall(SYS_futex, uaddr, futex_op, val, timeout, uaddr2, val3);

			break;
		}


		case __NR_sched_getaffinity:	// sched_getaffinity(204)
		{
			pid_t pid = *(pid_t*)arg1;
			size_t cpusetsize = *(size_t*)arg2;
			const cpu_set_t *mask = (const cpu_set_t *)arg3;

			*arg0 = syscall(n, pid, cpusetsize, mask);

			break;


		}


		case __NR_set_tid_address: 	// set_tid_address(218)
		{
			int *tidptr = (int*)*(long*)arg1;

			*arg0 = syscall(SYS_set_tid_address, tidptr);

			break;

		}

		case __NR_clock_gettime:	// clock_gettime(228)
		{
			clockid_t clk_id = *(clockid_t*)arg1;
			struct timespec *tp = (struct timespec*)arg2;

			*arg0 = clock_gettime(clk_id, tp);
			break;
		}


		case __NR_clock_getres:	// clock_getres(229)
		{
			clockid_t clk_id = *(clockid_t*)arg1;
			struct timespec *res = (struct timespec *)arg2;

			*arg0 = syscall(n, clk_id, res);

			break;
		}

		case __NR_exit_group:	// exit_group(231)
		{
			long status = *arg1;
			#ifdef PERFORMANCE_ANALYSIS
			RDTSC_STOP(t3,t4);
			clock_gettime(CLOCK_REALTIME, &end_rt);
			performance->total_time_real = end_rt.tv_sec +
			  end_rt.tv_nsec / BILLION;
			performance->total_time_real = performance->total_time_real -
			  performance->total_time_real_start;
			performance->total_time_end = ( ((long long)t3 << 32) | t4 );
			performance->total_pgm_cycles = performance->total_time_end -
			  performance->total_time_start;
			print_performance_metics (performance);
			#endif
			*arg0 = syscall(n, status);
			break;
		}

		case __NR_epoll_ctl: 	// epoll_ctl(233)
		{
			int epfd = *(int*)arg1;
			int op = *(int*)arg2;
			int fd = *(int*)arg3;
			struct epoll_event *event = (struct epoll_event*)*(long*)arg4;

			*arg0 = syscall(n, epfd, op, fd, event);

			break;
		}
		case __NR_utimes:	// utimes(235)
		{
			char *file_name = (char*)arg1;
			struct timeval *times = (struct timeval*)*(long*)arg2;

			*arg0 = syscall(n, file_name, times);


			break;
		}

		case __NR_openat:	// openat(257)
		{
			long dirfd = *arg1;
			char *path_name = (char*)arg2;
			long flags = *arg3;
			mode_t mode = *(mode_t*)arg4;

			*arg0 = syscall(n, dirfd, path_name, flags, mode);
			break;

		}


		case __NR_fchownat:	// fchownat(260)
		{
			long dirfd = *arg1;
			char *path_name = (char*)arg2;
			uid_t owner = *(uid_t*)arg3;
			gid_t group = *(gid_t*)arg4;
			long flags = *arg5;

			*arg0 = syscall(n, dirfd, path_name, owner, group, flags);

			break;

		}

		case __NR_futimesat:	// futimensat(261)
		{
			long fd = *arg1;
			char *path_name = (char*)arg2;
			struct timespec *times = (struct timespec*)*(long*)arg3;

			*arg0 = syscall(n, fd, path_name, times);

			break;

		}

		case __NR_unlinkat:	// unlinkat(263)
		{
			long dirfd = *arg1;
			char *path_name = (char*)arg2;
			long flags = *arg3;

			*arg0 = syscall(n, dirfd, path_name, flags);
			break;
		}
		case __NR_newfstatat:	// newfstatat(262)
		{
			long dirfd = *arg1;
			char *path_name = (char*)arg2;
			struct stat *st = (struct stat *)arg3;
			long flags = *arg4;

			*arg0 = syscall(n, dirfd, path_name, st, flags);

			break;
		}

		case __NR_readlinkat:	// readlinkat(267)
		{
			long dirfd = *arg1;
			char *path_name = (char*)arg2;
			char *buf = (char*)arg3;
			size_t buf_size = *(size_t*)arg4;

			*arg0 = syscall(n, dirfd, path_name, buf, buf_size);
			break;
		}

		case __NR_fchmodat:	// fchmodat(268)
		{
			long dirfd = *arg1;
			char *path_name = (char*)arg2;
			mode_t mode = *(mode_t*)arg3;
			long flags = *arg4;

			*arg0 = syscall(n, dirfd, path_name, mode, flags);

			break;
		}
		case __NR_faccessat:	// faccessat(269)
		{
			long dirfd = *arg1;
			char *path_name = (char*)arg2;
			long mode = *arg3;
			long flags = *arg4;

			*arg0 = syscall(n, dirfd, path_name, mode, flags);

			break;

		}


		case __NR_pselect6:	// pselect6(270)
		{
			int nfds = *(int*)arg1;
			fd_set *readfds = (fd_set*)*(long*)arg2;
			fd_set *writefds = (fd_set*)*(long*)arg3;
			fd_set *exceptfds = (fd_set*)*(long*)arg4;
			struct timespec *timeout = (struct timespec*)*(long*)arg5;
			sigset_t *sigmask = (sigset_t*)*(long*)arg6;

			*arg0 = pselect(nfds, readfds, writefds, exceptfds, timeout, sigmask);

			break;
		}


		case __NR_getdents64:		// getdents64(217)
		{
			unsigned int fd = *(unsigned int*)arg1;
			void *dirp = (void*)arg2;
			unsigned int count = *(unsigned int*)arg3;

			*arg0 = syscall(__NR_getdents64, fd, dirp, count);

			break;
		}

		case __NR_utimensat:	// utimensat(280)
		{
			long dirfd = *arg1;
			char *path_name = (char*)*(long*)arg2;
			struct timespec *times = (struct timespec*)*(long*)arg3;
			long flags = *arg4;

			*arg0 = syscall(n, dirfd, path_name, times, flags);

			break;
		}


		case __NR_epoll_pwait:	// epoll_pwait(281)
		{
			int epfd = *(int*)arg1;
			struct epoll_event *events = (struct epoll_event*)arg2;
			int maxevents = *(int*)arg3;
			int timeout = *(int*)arg4;
			const sigset_t *sigmask = (const sigset_t*)arg5; 
			size_t sigsetsize = *(size_t*)arg6;

			*arg0 = syscall(n, epfd, events, maxevents, timeout, NULL, sigsetsize);

			break;


		}

		case __NR_fallocate:	// fallocate(285)
		{
			long fd = *arg1;
			long mode = *arg2;
			off_t offset = *(off_t*)arg3;
			off_t len = *(off_t*)arg4;

			*arg0 = syscall(n, fd, mode, offset, len);

			break;
		}

		case __NR_accept4: 		// __NR_accept4(288)
		{
			int sockfd = *(int*)arg1;
			struct sockaddr *addr = (struct sockaddr*) arg2;
			socklen_t *addrlen = (socklen_t*)arg3;
			int flags = *(int*)arg4;

			*arg0 = syscall(n, sockfd, addr, addrlen, flags);

			break;
		}


		case __NR_eventfd2:	// eventfd(290)
		{
			unsigned int initval = *(unsigned int*)arg1;
			int flags = *(int*)arg2;

			*arg0 = syscall(n, initval, flags);

			break;
		}


		case __NR_epoll_create1:	// epoll_create1(291)
		{
			int flags = *(int*)arg1;

			*arg0 = syscall(n, flags);


			break;

		}

		case __NR_dup3:		// dup3(292)
		{
			long fd_old = *arg1;
			long fd_new = *arg2;
			long flags = *arg3;

			*arg0 = syscall(n, fd_old, fd_new, flags);

			break;
		}

		case __NR_pipe2:	// pipe2(293)
		{
			int *pipefd = (int*)arg1;
			int flags = *(int*)arg2;

			*arg0 = syscall(n, pipefd, flags);

			break;
		}

		case __NR_prlimit64:	// prlimit64(302)
		{
			pid_t pid = *(pid_t*)arg1;
			long resource = *arg2;
			struct rlimit *new_limit = (struct rlimit*)*(long*)arg3;
			struct rlimit *old_limit = (struct rlimit*)*(long*)arg4;

			*arg0 = syscall(n, pid, resource, new_limit, old_limit);

			break;
		}

		case __NR_syncfs:	// syncfs(306)
		{
			long fd = *arg1;

			*arg0 = syscall(n, fd);
			break;
		}

		case __NR_getrandom: 	// getrandom(318)
		{
			void *buf = (void*)arg1;
			size_t buflen = *(size_t*)arg2;
			unsigned int flags = *(unsigned int*)arg3;

			*arg0 = syscall(n, buf, buflen, flags);

			break;
		}

		default:
		{
			printf("syscall_unwrapper %d not denfined\n", n);
		}
	}
	if(*arg0 == -1){
		*arg0 = -errno;

		// we dont want SYS_futex to fill/pollute screen unnecessary by timeout message
		if(n != SYS_futex){
			//perror("(syscall_unwrapper)");
			//fprintf(stderr, "(syscall_unwrapper) n=%d errno = %ld\n\n", n, *arg0);
		}

	}
}
