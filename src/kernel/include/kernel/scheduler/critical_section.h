#ifndef _KERNEL_SCHEDULER_CRITICAL_SECTION_H_
#define _KERNEL_SCHEDULER_CRITICAL_SECTION_H_ 1
#include <kernel/scheduler/scheduler.h>
#include <kernel/types.h>



#define CRITICAL_SECTION scheduler_pause();for (u8 __tmp=0;!__tmp;scheduler_resume(),__tmp++)



#endif
