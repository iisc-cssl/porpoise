#include "atomic.h"

void __chk_fail(void)
{
	a_crash();
	for(;;);
}
