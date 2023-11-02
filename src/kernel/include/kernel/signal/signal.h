#ifndef _KERNEL_SIGNAL_SIGNAL_H_
#define _KERNEL_SIGNAL_SIGNAL_H_ 1
#include <kernel/isr/isr.h>
#include <kernel/mp/thread.h>
#include <kernel/types.h>



#define SIGNAL_TYPE_ZDE 0 // Zero Division Error
#define SIGNAL_TYPE_SCE 1 // System Call Exception
#define SIGNAL_TYPE_IOE 2 // Invalid Opcode Error
#define SIGNAL_TYPE_GPF 3 // General Protection Fault
#define SIGNAL_TYPE_PF 4 // Page Fault
#define SIGNAL_TYPE_FPE 5 // Floating Point Exception
#define SIGNAL_TYPE_USER 6 // First user-defined signal



typedef u32 signal_type_t;



void signal_send(thread_t* thread,isr_state_t* isr_state,signal_type_t type,const void* data,u32 size);



#endif
