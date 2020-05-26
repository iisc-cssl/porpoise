#include <stdlib.h>
#include <limits.h>
#include "atomics.h"

char *__realpath_chk(const char *filename, char *resolved, size_t resolved_len)
{
	if (resolved_len < PATH_MAX) a_crash();
	return realpath(filename, resolved);
}
