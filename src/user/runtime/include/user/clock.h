#ifndef _USER_CSPINlock_H_
#define _USER_CSPINlock_H_ 1
#include <user/types.h>



extern u64 cspinlock_cpu_frequency;



void cspinlock_init(void);



u64 cspinlock_get_ticks(void);



u64 cspinlock_get_time(void);



u64 cspinlock_ticks_to_time(u64 ticks);



#endif
