#ifndef _KERNEL_SIGNATURE_SIGNATURE_H_
#define _KERNEL_SIGNATURE_SIGNATURE_H_ 1
#include <kernel/mmap/mmap.h>



void signature_verify_kernel(void);



_Bool signature_verify_module(const char* name,const mmap_region_t* region,_Bool* is_tainted);



_Bool signature_verify_library(const char* name,const mmap_region_t* region);



_Bool signature_require_signatures(void);



_Bool signature_is_kernel_tainted(void);



#endif
