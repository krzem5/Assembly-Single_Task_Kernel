#ifndef _KERNEL_MSR_MSR_H_
#define _KERNEL_MSR_MSR_H_ 1
#include <kernel/types.h>



u8 msr_get_apic_id(void);



void msr_enable_apic(void);



void msr_enable_rdtsc(void);



void msr_set_fs_base(u64 fs_base);



void msr_set_gs_base(u64 gs_base,bool is_alternate_gs);



void msr_write(u32 msr,u64 value);



#endif
