#ifndef _KERNEL_SIGNATURE_SIGNATURE_H_
#define _KERNEL_SIGNATURE_SIGNATURE_H_ 1
#include <kernel/mmap/mmap.h>



void signature_verify_kernel(void);



_Bool signature_verify_module(const char* name,const mmap_region_t* region);



#endif
