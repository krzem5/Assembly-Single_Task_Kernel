#ifndef _KERNEL_SYSCALL_SYSCALL_H_
#define _KERNEL_SYSCALL_SYSCALL_H_ 1
#include <kernel/error/error.h>
#include <kernel/types.h>



typedef error_t (*syscall_callback_t)();



typedef struct _SYSCALL_TABLE{
	const char* name;
	const syscall_callback_t* functions;
	u32 function_count;
	u32 index;
} syscall_table_t;



extern const syscall_callback_t _syscall_kernel_functions[];
extern const u64 _syscall_kernel_count;



u32 syscall_create_table(const char* name,const syscall_callback_t* functions,u32 function_count);



_Bool syscall_update_table(u32 index,const syscall_callback_t* functions,u32 function_count);



u64 syscall_get_user_pointer_max_length(const void* ptr);



u64 syscall_get_string_length(const void* ptr);



void syscall_enable(void);



#endif
