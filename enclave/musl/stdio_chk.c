#include <stdio.h>
#include <wchar.h>
#include "atomic.h"
#include "libc.h"
#include "stdio_impl.h"

char *__fgets_chk(char *s, size_t size, int n, FILE *f)
{
	if ((size_t)n > size) a_crash();
	return fgets(s, n, f);
}

wchar_t *__fgetws_chk(wchar_t *s, size_t size, int n, FILE *f)
{
	if ((size_t)n > size) a_crash();
	return fgetws(s, n, f);
}

size_t __fread_chk(void *restrict destv, size_t destvlen, size_t size, size_t nmemb, FILE *restrict f)
{
	if (size != 0 && (size * nmemb) / size != nmemb) a_crash();
	if (size * nmemb > destvlen) a_crash();
	return fread(destv, size, nmemb, f);
}

char *__gets_chk(char *buf, size_t size)
{
	char *ret = buf;
	int c;
	FLOCK(stdin);
	if (!size) return NULL;
	for(;size;buf++,size--) {
		c = getc(stdin);
		if ((c == EOF && feof(stdin)) || c == '\n') {
			*buf = 0;
			FUNLOCK(stdin);
			return ret;
		}
		if (c == EOF) {
			FUNLOCK(stdin);
			return NULL;
		}
		*buf = c;
	}
	a_crash();
}

weak_alias(__fgets_chk, __fgets_unlocked_chk);
weak_alias(__fgetws_chk, __fgetws_unlocked_chk);
weak_alias(__fread_chk, __fread_unlocked_chk);
