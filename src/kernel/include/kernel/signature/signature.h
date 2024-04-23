#ifndef _KERNEL_SIGNATURE_SIGNATURE_H_
#define _KERNEL_SIGNATURE_SIGNATURE_H_ 1
#include <kernel/mmap/mmap.h>



void signature_verify_kernel(void);



bool signature_verify_module(const char* name,const mmap_region_t* region,bool* is_tainted);



bool signature_verify_user(const char* name,const mmap_region_t* region);



bool signature_is_kernel_tainted(void);



#endif
