#ifndef _KERNEL_SYSCALL_SYSCALL_H_
#define _KERNEL_SYSCALL_SYSCALL_H_ 1
#include <kernel/isr/isr.h>
#include <kernel/types.h>



typedef struct _SYSCALL_REG_STATE{
	u64 rax;
	u64 rbx;
	u64 rcx;
	u64 rdx;
	u64 rsi;
	u64 rdi;
	u64 rbp;
	u64 r8;
	u64 r9;
	u64 r10;
	u64 r11;
	u64 r12;
	u64 r13;
	u64 r14;
	u64 r15;
} syscall_reg_state_t;



typedef void (*syscall_callback_t)(syscall_reg_state_t*);



typedef struct _SYSCALL_TABLE{
	const char* name;
	const syscall_callback_t* functions;
	u32 function_count;
	u32 index;
} syscall_table_t;



void syscall_init(void);



u32 syscall_create_table(const char* name,const syscall_callback_t* functions,u32 function_count);



u64 syscall_get_user_pointer_max_length(u64 address);



u64 syscall_get_string_length(u64 address);



void syscall_enable(void);



#endif
