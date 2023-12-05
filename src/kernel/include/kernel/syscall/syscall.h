#ifndef _KERNEL_SYSCALL_SYSCALL_H_
#define _KERNEL_SYSCALL_SYSCALL_H_ 1
#include <kernel/isr/isr.h>
#include <kernel/types.h>



typedef void (*syscall_callback_t)(isr_state_t*);



void syscall_init(void);



u32 syscall_create_table(const char* name,const syscall_callback_t* functions,u32 function_count);



u64 syscall_get_user_pointer_max_length(u64 address);



u64 syscall_get_string_length(u64 address);



void syscall_enable(void);



#endif
