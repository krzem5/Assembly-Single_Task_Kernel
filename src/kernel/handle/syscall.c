#include <kernel/handle/handle.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/util.h>



typedef struct _USER_HANDLE_TYPE_DATA{
	char name[HANDLE_NAME_LENGTH];
	u64 count;
	u64 active_count;
} user_handle_type_data_t;



void syscall_handle_get_type_count(syscall_registers_t* regs){
	regs->rax=0;
}



void syscall_handle_get_type(syscall_registers_t* regs){
	if (regs->rdx!=sizeof(user_handle_type_data_t)||!syscall_sanatize_user_memory(regs->rsi,regs->rdx)){
		regs->rax=0;
		return;
	}
	regs->rax=0;
}
