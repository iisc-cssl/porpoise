#include <stdlib.h>
#include <string.h>

/* Multiple Definitions. Also in SGX SDK 
char *strdup(const char *s)
{
	size_t l = strlen(s);
	char *d = malloc(l+1);
	if (!d) return NULL;
	return memcpy(d, s, l+1);
}
*/
