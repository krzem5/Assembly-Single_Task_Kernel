#ifndef _KERNEL_SIGNAL_SIGNAL_H_
#define _KERNEL_SIGNAL_SIGNAL_H_ 1
#include <kernel/isr/isr.h>
#include <kernel/mp/thread.h>
#include <kernel/signal/_signal_types.h>
#include <kernel/types.h>



#define SIGNAL_TYPE_NONE 0 // Zero Division Error
#define SIGNAL_TYPE_ZDE 1 // Zero Division Error
#define SIGNAL_TYPE_SCE 2 // System Call Exception
#define SIGNAL_TYPE_IOE 3 // Invalid Opcode Error
#define SIGNAL_TYPE_GPF 4 // General Protection Fault
#define SIGNAL_TYPE_PF 5 // Page Fault
#define SIGNAL_TYPE_FPE 6 // Floating Point Exception
#define SIGNAL_TYPE_USER 7 // First user-defined signal



void signal_send(thread_t* thread,isr_state_t* isr_state,signal_type_t type,u64 arg);



#endif
