#define __SYSCALL_LL_E(x) (x)
#define __SYSCALL_LL_O(x) (x)

/*
 * arch/x86_64/syscall_arch.h
 *
 * I wanted to call __syscall instead of __syscall_wrap but I am unable to do so, 
 * due to confict in declaration and macro exapansion.
 *
 * __syscall_wrap is not part of musl-libc. It is implemented seprately
 * as a shim layer to translate logical processor from enclave to user mode.
 */

long __syscall_wrap(long n, long a1, long a2, long a3, long a4, long a5, long a6);


static __inline long __syscall0(long n)
{
	return __syscall_wrap(n, 0, 0, 0, 0, 0, 0);
}

static __inline long __syscall1(long n, long a1)
{
	return __syscall_wrap(n, a1, 0, 0, 0, 0, 0);
}

static __inline long __syscall2(long n, long a1, long a2)
{
	return __syscall_wrap(n, a1, a2, 0, 0, 0, 0);
}

static __inline long __syscall3(long n, long a1, long a2, long a3)
{
	return __syscall_wrap(n, a1, a2, a3, 0, 0, 0);
}

static __inline long __syscall4(long n, long a1, long a2, long a3, long a4)
{
	return __syscall_wrap(n, a1, a2, a3, a4, 0, 0);
}

static __inline long __syscall5(long n, long a1, long a2, long a3, long a4, long a5)
{
	return __syscall_wrap(n, a1, a2, a3, a4, a5, 0);
}

static __inline long __syscall6(long n, long a1, long a2, long a3, long a4, long a5, long a6)
{
	return __syscall_wrap(n, a1, a2, a3, a4, a5, a6);
}

/*
#define VDSO_USEFUL
#define VDSO_CGT_SYM "__vdso_clock_gettime"
#define VDSO_CGT_VER "LINUX_2.6"
#define VDSO_GETCPU_SYM "__vdso_getcpu"
#define VDSO_GETCPU_VER "LINUX_2.6"
*/
