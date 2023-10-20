#include <kernel/handle/handle.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/util.h>



#define HANDLE_INVALID 0xffffffffffffffffull



void syscall_handle_get_type_by_name(syscall_registers_t* regs){
	regs->rax=0;
	if (!syscall_sanatize_user_memory(regs->rdi,regs->rsi)){
		return;
	}
	const char* name=(void*)(regs->rdi);
	HANDLE_FOREACH(HANDLE_TYPE_HANDLE){
		handle_descriptor_t* handle_descriptor=handle->object;
		for (u64 i=0;i<regs->rsi;i++){
			if (convert_lowercase(name[i])!=convert_lowercase(handle_descriptor->name[i])){
				goto _check_next_handle;
			}
		}
		if (!handle_descriptor->name[regs->rsi]){
			regs->rax=handle_descriptor->rb_node.key;
			return;
		}
_check_next_handle:
	}
}



void syscall_handle_get_type_count(syscall_registers_t* regs){
	regs->rax=0;
}



void syscall_handle_get_type(syscall_registers_t* regs){
	regs->rax=0;
}
