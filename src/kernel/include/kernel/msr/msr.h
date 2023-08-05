#ifndef _KERNEL_MSR_MSR_H_
#define _KERNEL_MSR_MSR_H_ 1
#include <kernel/types.h>



u8 msr_get_apic_id(void);



void msr_set_fs_base(void* fs_base);



void msr_set_gs_base(void* gs_base,_Bool is_alternate_gs);



void msr_enable_simd(void);



void msr_enable_rdtsc(void);



void msr_enable_fsgsbase(void);



#endif
