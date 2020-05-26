#define _GNU_SOURCE
#include <stdio.h>
#include <stdarg.h>

int __asprintf_chk(char **restrict s, int flag, const char *restrict fmt, ...)
{
	int ret;
	va_list ap;
	va_start(ap, fmt);
	ret = vasprintf(s, fmt, ap);
	va_end(ap);
	return ret;
}

int __vasprintf_chk(char **restrict s, int flag, const char *restrict fmt, va_list ap)
{
	return vasprintf(s, fmt, ap);
}
