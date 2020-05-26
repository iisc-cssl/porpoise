
/*
 * src/internal/x86_64/syscall.c
 *
 * Kripa Shanker <kripashanker@iisc.ac.in>
 *
 * __syscall_wrap is not part of musl-libc. It is implemented seprately
 * as a shim layer to translate logical processor from enclave to user mode.
 *
 */

long __syscall_wrap(long n, long a1, long a2, long a3, long a4, long a5, long a6);

long __syscall(long n, long a1, long a2, long a3, long a4, long a5, long a6)
{
	return __syscall_wrap(n, a1, a2, a3, a4, a5, a6);
}
