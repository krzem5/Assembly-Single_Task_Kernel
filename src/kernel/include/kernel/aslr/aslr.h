#ifndef _KERNEL_ASLR_ASLR_H_
#define _KERNEL_ASLR_ASLR_H_ 1
#include <kernel/types.h>



extern u64 aslr_module_base;
extern u64 aslr_module_size;



u64 aslr_generate_address(u64 min,u64 max);



void KERNEL_NORETURN aslr_reloc_kernel(void (*KERNEL_NORETURN next_stage_callback)(void));



void KERNEL_NORETURN _aslr_adjust_rip(void* arg,void* callback);



#endif
