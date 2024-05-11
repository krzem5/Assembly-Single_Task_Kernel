#ifndef _KERNEL_SIGNATURE_SIGNATURE_H_
#define _KERNEL_SIGNATURE_SIGNATURE_H_ 1
#include <kernel/mmap/mmap.h>
#include <kernel/types.h>



#define SIGNATURE_TAINT_FLAG_KERNEL 1
#define SIGNATURE_TAINT_FLAG_USER 2



void signature_verify_kernel(void);



bool signature_verify_module(const char* name,const mmap_region_t* region,bool* is_tainted);



bool signature_verify_user(const char* name,const mmap_region_t* region);



u32 signature_get_taint_flags(void);



#endif
