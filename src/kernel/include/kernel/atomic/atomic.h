#ifndef _KERNEL_ATOMIC_ATOMIC_H_
#define _KERNEL_ATOMIC_ATOMIC_H_ 1
#include <kernel/types.h>



u32 atomic_get(u32* var);



u32 atomic_set(u32* var,u32 set);



u32 atomic_cas(u32* var,u32 test,u32 set);



#endif
