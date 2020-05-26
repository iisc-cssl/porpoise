#include <sys/mman.h>
#include "syscall.h"

/*
 * SGXSDK: Removed due to multiple defination
int msync(void *start, size_t len, int flags)
{
	return syscall_cp(SYS_msync, start, len, flags);
}
*/
