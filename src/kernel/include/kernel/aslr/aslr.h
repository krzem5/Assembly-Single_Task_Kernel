#ifndef _KERNEL_ASLR_ASLR_H_
#define _KERNEL_ASLR_ASLR_H_ 1
#include <kernel/types.h>



extern u64 aslr_module_base;



void aslr_reloc_kernel(void);



#endif
