#include <wchar.h>
#include <stdlib.h>
#include "atomic.h"

size_t __mbsnrtowcs_chk(wchar_t *restrict wcs, const char **restrict src, size_t n, size_t wn, mbstate_t *restrict st, size_t wcslen)
{
	if (wn > wcslen) a_crash();
	return msbnrtowcs(wcs, src, n, wn, st);
}

size_t __mbsrtowcs_chk(wchar_t *restrict ws, const char **restrict src, size_t wn, mbstate_t *restrict st, size_t wslen)
{
	if (wn > wslen) a_crash();
	return mbsrtowcs(ws, src, wn, st);
}

size_t __mbstowcs_chk(wchar_t *restrict ws, const char *restrict s, size_t wn, size_t wslen)
{
	if (wn > wslen) a_crash();
	return mbstowcs(ws, s, wn);
}

size_t __wcrtomb_chk(char *restrict s, wchar_t wc, mbstate_t *restrict st, size_t slen)
{
	if (slen < MB_CUR_MAX) a_crash();
	return wcrtomb(s, wc, st);
}

size_t __wcsnrtombs_chk(char *restrict dst, const wchar_t **restrict wcs, size_t wn, size_t n, mbstate_t *restrict st, size_t dstlen)
{
	if (n > dstlen) a_crash();
	return wcsnrtombs(dst, wcs, wn, n, st);
}

size_t __wcstombs_chk(char *restrict s, const wchar_t *restrict ws, size_t n, size_t slen)
{
	if (n > slen) a_crash();
	return wcstombs(s, ws, n);
}

int __wctomb_chk(char *s, wchar_t wc, size_t slen)
{
	if (slen < MB_CUR_MAX) a_crash();
	return wctomb(s, wc);
}
