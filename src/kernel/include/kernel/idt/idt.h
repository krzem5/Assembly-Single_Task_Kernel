#ifndef _KERNEL_IDT_IDT_H_
#define _KERNEL_IDT_IDT_H_ 1
#include <kernel/types.h>



void idt_enable(void);



void _idt_set_data_pointer(u64 addr);



#endif
