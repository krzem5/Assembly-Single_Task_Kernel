#ifndef _KERNEL_ISR_ISR_H_
#define _KERNEL_ISR_ISR_H_ 1
#include <kernel/types.h>



u8 isr_allocate(void);



_Bool isr_wait(u8 irq);



#endif
