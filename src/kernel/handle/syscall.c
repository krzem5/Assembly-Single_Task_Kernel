#include <kernel/handle/handle.h>
#include <kernel/isr/isr.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/util.h>



#define HANDLE_INVALID 0xffffffffffffffffull

#define HANDLE_DATA_NAME_LENGTH 64



typedef struct _USER_HANDLE_DATA{
	char name[HANDLE_DATA_NAME_LENGTH];
	u64 count;
	u64 active_count;
	u16 type;
} user_handle_data_t;



void syscall_handle_get_type(isr_state_t* regs){
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



void syscall_handle_get_handle(isr_state_t* regs){
	handle_descriptor_t* handle_descriptor=handle_get_descriptor(regs->rdi);
	if (!handle_descriptor||regs->rsi==HANDLE_INVALID){
		regs->rax=HANDLE_INVALID;
		return;
	}
	rb_tree_node_t* node=rb_tree_lookup_increasing_node(&(handle_descriptor->tree),regs->rsi);
	regs->rax=(node?node->key:HANDLE_INVALID);
}



void syscall_handle_get_data(isr_state_t* regs){
	if (regs->rdx!=sizeof(user_handle_data_t)||!syscall_sanatize_user_memory(regs->rsi,regs->rdx)){
		return;
	}
	handle_t* handle=handle_lookup_and_acquire(regs->rdi,HANDLE_TYPE_HANDLE);
	if (!handle){
		regs->rax=0;
		return;
	}
	user_handle_data_t* out=(void*)(regs->rsi);
	handle_descriptor_t* handle_descriptor=handle->object;
	strcpy_lowercase(out->name,handle_descriptor->name,HANDLE_DATA_NAME_LENGTH);
	out->count=handle_descriptor->count;
	out->active_count=handle_descriptor->active_count;
	out->type=handle_descriptor->rb_node.key;
	regs->rax=1;
}
