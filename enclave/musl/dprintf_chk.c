#include <stdio.h>
#include <stdarg.h>

int __dprintf_chk(int fd, int flag, const char *fmt, ...)
{
	int ret;
	va_list ap;
	va_start(ap, fmt);
	ret = vdprintf(fd, fmt, ap);
	va_end(ap);
	return ret;
}

int __vdprintf(int fd, int flag, const char *fmt, va_list ap)
{
	return vdprintf(fd, fmt, ap);
}
