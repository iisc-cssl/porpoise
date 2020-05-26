#include "syscall.h"

hidden long __syscall_cp_asm_wrap(void *cancel, long nr, long a1, long a2, long a3, long a4, long a5, long a6){
	return __syscall_wrap(nr, a1, a2, a3, a4, a5, a6);
}
