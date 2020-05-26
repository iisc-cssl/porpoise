#include <wchar.h>
#include <stdio.h>
#include <stdarg.h>
#include "atomic.h"

int __fwprintf_chk(FILE *restrict f, int flag, const wchar_t *restrict fmt, ...)
{
	int ret;
	va_list ap;
	va_start(ap, fmt);
	ret = vfwprintf(f, fmt, ap);
	va_end(ap);
	return ret;
}

int __swprintf_chk(wchar_t *restrict s, size_t n, int flag, size_t slen, const wchar_t *restrict fmt, ...)
{
	int ret;
	va_list ap;
	if (n > slen) a_crash();
	va_start(ap, fmt);
	ret = vswprintf(s, n, fmt, ap);
	va_end(ap);
	return ret;
}

int __wprintf_chk(int flag, const wchar_t *restrict fmt, ...)
{
	int ret;
	va_list ap;
	va_start(ap, fmt);
	ret = vfwprintf(stdout, fmt, ap);
	va_end(ap);
	return ret;
}

int __vfwprintf_chk(FILE *f, int flag, const wchar_t *restrict fmt, va_list ap)
{
	return vfwprintf(f, fmt, ap);
}

int __vswprintf_chk(wchar_t *restrict s, size_t n, int flag, size_t slen, const wchar_t *restrict fmt, va_list ap)
{
	if (n > slen) a_crash();
	return vswprintf(s, n, fmt, ap);
}

int __vwprintf_chk(int flag, const wchar_t *restrict fmt, va_list ap)
{
	return vfwprintf(stdout, fmt, ap);
}
