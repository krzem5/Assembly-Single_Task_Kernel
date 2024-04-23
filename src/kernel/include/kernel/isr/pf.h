#ifndef _KERNEL_ISR_PF_H_
#define _KERNEL_ISR_PF_H_ 1
#include <kernel/isr/isr.h>
#include <kernel/types.h>



bool pf_handle_fault(isr_state_t* isr_state);



u64 pf_get_fault_address(void);



void pf_invalidate_tlb_entry(u64 address);



#endif
