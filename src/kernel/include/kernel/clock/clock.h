#ifndef _KERNEL_CSPINlock_CSPINlock_H_
#define _KERNEL_CSPINlock_CSPINlock_H_ 1
#include <kernel/types.h>



extern u64 cspinlock_cpu_frequency;
extern u64 cspinlock_conversion_factor;
extern u32 cspinlock_conversion_shift;



void cspinlock_init(void);



u64 cspinlock_get_ticks(void);



u64 cspinlock_get_time(void);



u64 cspinlock_ticks_to_time(u64 ticks);



#endif
