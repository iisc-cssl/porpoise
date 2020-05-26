#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <wchar.h>
#include "atomic.h"

ssize_t __read_chk(int fd, void *buf, size_t count, size_t buflen)
{
	if (count > buflen) a_crash();
	return read(fd, buf, count);
}

ssize_t __readlink_chk(const char *path, void *buf, size_t bufsize, size_t buflen)
{
	if (bufsize > buflen) a_crash();
	return readlink(path, buf, bufsize);
}

ssize_t __readlinkat_chk(int fd, const char *path, void *buf, size_t bufsize, size_t buflen)
{
	if (bufsize > buflen) a_crash();
	return readlinkat(fd, path, buf, bufsize);
}

ssize_t __recv_chk(int fd, void *buf, size_t len, size_t buflen, int flags)
{
	if (len > buflen) a_crash();
	return recv(fd, buf, len, flags);
}

ssize_t __recvfrom_chk(int fd, void *buf, size_t len, size_t buflen, int flags, struct sockaddr *addr, socklen_t *alen)
{
	if (len > buflen) a_crash();
	return recvfrom(fd, buf, len, flags, addr, alen);
}

int __open_2(const char *path, int flag)
{
	if (flag & O_CREAT) a_crash();
	return open(path, flag);
}

int __open64_2(const char *path, int flag)
{
	return __open_2(path, flag);
}

int __openat_2(int fd, const char *path, int flag)
{
	if (flag & O_CREAT) a_crash();
	return openat(fd, path, flag);
}

int __openat64_2(int fd, const char *path, int flag)
{
	return __openat_2(fd, path, flag);
}

int __poll_chk(struct pollfd *fds, nfds_t n, int timeout,  size_t fdslen)
{
	if (fdslen / sizeof(fds[0]) < n) a_crash();
	return poll(fds, n, timeout);
}

ssize_t __pread_chk(int fd, void *buf, size_t size, size_t ofs, size_t buflen)
{
	if (size > buflen) a_crash();
	return pread(fd, buf, size, ofs);
}

ssize_t __pread64_chk(int fd, void *buf, size_t size, size_t ofs, size_t buflen)
{
	return __pread_chk(fd, buf, size, ofs, buflen);
}

char *__getcwd_chk(char *buf, size_t len, size_t buflen)
{
	if (len > buflen) a_crash();
	return getcwd(buf, len);
}

int __gethostname_chk(char *name, size_t len, size_t namelen)
{
	if (len > namelen) a_crash();
	return gethostname(name, len);
}

long __fdelt_chk(long d)
{
	if (d < 0 || d >= FD_SETSIZE) a_crash();
	return d / (8 * sizeof(d));
}

int __ttyname_r_chk(int fd, char *name, size_t size, size_t namelen)
{
	if (size > namelen) a_crash();
	return ttyname_r(fd, name, size);
}

char *__stpcpy_chk(char *d, const char *s, size_t dlen)
{
	size_t slen = strnlen(s, dlen) + 1;
	if (slen > dlen) a_crash();
	return stpcpy(d, s);
}

char *__stpncpy_chk(char *d, const char *s, size_t n, size_t dlen)
{
	if (n > dlen) a_crash();
	return stpncpy(d, s, n);
}

wchar_t *__wcpcpy_chk(wchar_t *restrict d, const wchar_t *restrict s, size_t dlen)
{
	size_t slen = strnlen(s, dlen) + 1;
	if (slen > dlen) a_crash();
	return wcpcpy(d, s);
}

wchar_t *__wcpncpy_chk(wchar_t *restrict d, const wchar_t *restrict s, size_t n, size_t dlen)
{
	if (n > dlen) a_crash();
	return wcpncpy(d, s, n);
}

int __getgroups_chk(int count, gid_t list[], size_t len)
{
	if (count > len / sizeof(list[0])) a_crash();
	return getgroups(count, list);
}
