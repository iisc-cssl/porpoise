#ifndef _SGX_PERF_H
#define _SGX_PERF_H

#include <sys/time.h>
#include <sys/resource.h>

#if defined(__cplusplus)
extern "C" {
#endif
void sgx_perf_init();
void sgx_perf_terminate();
void perf_run();
#if defined(__cplusplus)
}
#endif

#endif /* !_SGX_PERF_H */
