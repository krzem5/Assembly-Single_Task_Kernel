#ifndef _KERNEL_FPU_FPU_H_
#define _KERNEL_FPU_FPU_H_ 1
#include <kernel/types.h>



extern u32 fpu_state_size;



void fpu_enable(void);



void fpu_init(void* fpu_state);



void fpu_save(void* fpu_state);



void fpu_restore(void* fpu_state);



#endif
