#ifndef _KERNEL_ISR_ISR_H_
#define _KERNEL_ISR_ISR_H_ 1
#include <kernel/isr/_isr_types.h>
#include <kernel/mp/event.h>
#include <kernel/types.h>



#define IRQ_EVENT(irq) (irq_events[(irq)-33])
#define IRQ_HANDLER(irq) (irq_handlers[(irq)-33])
#define IRQ_HANDLER_CTX(irq) (irq_handler_contexts[(irq)-33])



typedef void (*irq_handler_t)(void*);



extern event_t* irq_events[223];
extern KERNEL_ATOMIC irq_handler_t irq_handlers[223];
extern void* KERNEL_ATOMIC irq_handler_contexts[223];



u8 isr_allocate(void);



#endif
