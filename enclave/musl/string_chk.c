#include <string.h>
#include <wchar.h>
#include "atomic.h"

void *__memcpy_chk(void *restrict dest, const void *restrict src, size_t n, size_t destlen)
{
	if (n > destlen) a_crash();
	return memcpy(dest, src, n);
}

void *__memmove_chk(void *restrict dest, const void *restrict src, size_t n, size_t destlen)
{
	if (n > destlen) a_crash();
	return memmove(dest, src, n);
}

void *__memset_chk(void *dest, int c, size_t n, size_t destlen)
{
	if (n > destlen) a_crash();
	return memset(dest, c, n);
}

char *__strcat_chk(char *restrict dest, const char *restrict src, size_t destlen)
{
	size_t tot;
	tot = strnlen(src, destlen);
	if (tot > SIZE_MAX - strnlen(dest, destlen) - 1) a_crash();
	tot += strnlen(dest, destlen) + 1;
	if (tot > destlen) a_crash();
	return strcat(dest, src);
}

char *__strncat_chk(char *restrict dest, const char *restrict src, size_t n, size_t destlen)
{
	size_t tot;
	tot = strnlen(dest, destlen);
	if (tot > SIZE_MAX - strnlen(src, n) - 1) a_crash();
	tot += strnlen(src, n) + 1;
	if (tot > destlen) a_crash();
	return strncat(dest, src, n);
}

char *__strncpy_chk(char *restrict dest, const char *restrict src, size_t n, size_t destlen)
{
	if (n > destlen) a_crash();
	return strncpy(dest, src, n);
}

wchar_t *__wcscat_chk(wchar_t *restrict dest, const wchar_t *src, size_t destlen)
{
	size_t tot;
	tot = wcsnlen(src, destlen);
	if (tot > SIZE_MAX - wcsnlen(dest, destlen) - 1) a_crash();
	tot += wcsnlen(dest, destlen) + 1;
	if (tot > destlen) a_crash();
	return wcscat(dest, src);
}

wchar_t *__wcscpy_chk(wchar_t *restrict dest, const wchar_t *restrict src, size_t destlen)
{
	size_t srclen = wcsnlen(src, destlen) + 1;
	if (srclen > destlen) a_crash();
	return wcscpy(dest, src);
}

wchar_t *__wcsncat_chk(wchar_t *restrict dest, const wchar_t *restrict src, size_t n, size_t destlen)
{
	size_t tot;
	tot = wcsnlen(dest, destlen);
	if (tot > SIZE_MAX - wcsnlen(src, n) - 1) a_crash();
	tot += wcsnlen(dest, destlen) + 1;
	if (tot > destlen) a_crash();
	return wcsncat(dest, src, n);
}

wchar_t *__wcsncpy_chk(wchar_t *restrict dest, const wchar_t *restrict src, size_t n, size_t destlen)
{
	if (n > destlen) a_crash();
	return wcsncpy(dest, src, n);
}

wchar_t *__wmemcpy_chk(wchar_t *restrict d, const wchar_t *restrict s, size_t n, size_t dlen)
{
	if (n > dlen) a_crash();
	return wmemcpy(d, s, n);
}

wchar_t *__wmemmove_chk(wchar_t *d, const wchar_t *s, size_t n, size_t dlen)
{
	if (n > dlen) a_crash();
	return wmemmove(d, s, n);
}

wchar_t *__wmemset_chk(wchar_t *d, wchar_t c, size_t n, size_t dlen)
{
	if (n > dlen) a_crash();
	return wmemset(d, c, n);
}
