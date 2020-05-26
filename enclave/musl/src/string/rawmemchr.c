#include <string.h>
#include "libc.h"

void *rawmemchr(const void *s, int c)
{
       register const unsigned char *r = s;

       while (*r != ((unsigned char)c)) ++r;

       return (void *) r;      /* silence the warning */
}
weak_alias(rawmemchr, __rawmemchr);


