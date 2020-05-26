#define _GNU_SOURCE
#include <unistd.h>
#include "libc.h"

weak int getpagesize(void)
{
	return PAGE_SIZE;
}
