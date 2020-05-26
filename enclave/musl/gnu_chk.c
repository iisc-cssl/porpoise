#define _GNU_SOURCE
#include <string.h>
#include <poll.h>
#include <stddef.h>
#include "atomic.h"
#include "atomic.h"

int __ppoll_chk(struct pollfd *fds, nfds_t n, const struct timespec *to, const sigset_t *mask, size_t fdslen)
{
	if (fdslen / sizeof(*fds) < n) a_crash();
	return ppoll(fds, n, to, mask);
}

void *__mempcpy_chk(void *dest, const void *src, size_t n, size_t destlen)
{
	if (destlen < n) a_crash();
	return mempcpy(dest, src, n);
}
