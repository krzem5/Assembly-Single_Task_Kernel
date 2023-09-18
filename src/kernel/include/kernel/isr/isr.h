#ifndef _KERNEL_ISR_ISR_H_
#define _KERNEL_ISR_ISR_H_ 1
#include <kernel/isr/_isr_types.h>
#include <kernel/thread/thread.h>
#include <kernel/types.h>



#define IRQ_EVENT(irq) (irq_events[(irq)-33])



extern event_t* irq_events[223];



void isr_init(void);



u8 isr_allocate(void);



#endif
