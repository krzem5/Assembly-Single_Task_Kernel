#ifndef _KERNEL_IDT_IDT_H_
#define _KERNEL_IDT_IDT_H_ 1
#include <kernel/types.h>



void idt_init(void);



void idt_set_entry(u8 index,void* callback,u8 ist,u8 flags);



void idt_enable(void);



#endif
