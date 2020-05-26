#include <setjmp.h>
#include <signal.h>

_Noreturn void __longjmp_chk(sigjmp_buf buf, int ret)
{
	longjmp((jmp_buf)buf, ret);
}
