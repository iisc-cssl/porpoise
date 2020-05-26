#include <unistd.h>
#include <atomic.h>

size_t __confstr_chk(int name, char *buf, size_t len, size_t buflen)
{
	if (buflen < len) a_crash();
	return confstr(name, buf, len);
}
