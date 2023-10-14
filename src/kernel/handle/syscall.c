#include <kernel/handle/handle.h>
#include <kernel/mp/thread.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/util.h>



void syscall_handle_get_type_count(syscall_registers_t* regs){
	regs->rax=handle_type_count;
}



void syscall_handle_get_type(syscall_registers_t* regs){
	if (regs->rdx!=sizeof(handle_user_type_data_t)||!syscall_sanatize_user_memory(regs->rsi,regs->rdx)||regs->rdi>=handle_type_count){
		regs->rax=0;
		return;
	}
	const handle_type_data_t* type_data=handle_type_data+regs->rdi;
	handle_user_type_data_t* out=(void*)(regs->rsi);
	memcpy(out->name,type_data->name,HANDLE_NAME_LENGTH);
	out->count=type_data->count;
	regs->rax=1;
}
