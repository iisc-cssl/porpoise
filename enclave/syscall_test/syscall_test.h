
#ifndef _SYSCALL_TEST_H_
#define _SYSCALL_TEST_H_

struct test_t {
	unsigned int tcnt;
	int needs_tmpdir:1;

	void (*setup)(void);
	void (*cleanup)(void);
	
	void (*test)(unsigned int test_nr);
	void (*test_all)(void);
};

#if defined(__cplusplus)
extern "C" {
#endif


#if defined(__cplusplus)
}
#endif

#endif /* !_SYSCALL_TEST_H_ */

