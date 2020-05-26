#include <stdio.h>
#include <stdarg.h>

int __fprintf_chk(FILE *f, int flag, const char *fmt, ...)
{
	int ret;
	va_list ap;
	va_start(ap, fmt);
	ret = vfprintf(f, fmt, ap);
	va_end(ap);
	return ret;
}

int __printf_chk(int flag, const char *fmt, ...)
{
	int ret;
	va_list ap;
	va_start(ap, fmt);
	ret = vfprintf(stdout, fmt, ap);
	va_end(ap);
	return ret;
}
