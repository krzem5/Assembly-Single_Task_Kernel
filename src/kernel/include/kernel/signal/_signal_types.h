#ifndef _KERNEL_SIGNAL__SIGNAL_TYPES_H_
#define _KERNEL_SIGNAL__SIGNAL_TYPES_H_ 1
#include <kernel/isr/_isr_types.h>
#include <kernel/types.h>



typedef u32 signal_type_t;



typedef struct _SIGNAL_STATE{
	signal_type_t type;
	u64 arg;
	isr_state_t old_gpr_state;
	void* old_fpu_state;
} signal_state_t;



#endif
