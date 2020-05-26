#include <unistd.h>

int __ttyname_r_chk(int fd, char *name, size_t size, size_t namelen)
{
	if (size > namelen) a_crash();
	return ttyname_r(fd, name, size);
}
