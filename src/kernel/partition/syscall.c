#include <kernel/partition/partition.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>



typedef struct _USER_PARTITION{
	u8 flags;
	u8 type;
	u8 index;
	u64 first_block_index;
	u64 last_block_index;
	char name[48];
	u32 drive_index;
} user_partition_t;



static void _copy_string(char* dst,const char* src,u8 maxlength){
	u8 i=0;
	for (;i<maxlength-1&&src[i];i++){
		dst[i]=src[i];
	}
	dst[i]=0;
}



void syscall_partition_get(syscall_registers_t* regs){
	if (regs->rdx!=sizeof(user_partition_t)||!syscall_sanatize_user_memory(regs->rsi,regs->rdx)){
		regs->rax=0;
		return;
	}
	const partition_t* partition=partition_get(regs->rdi);
	if (!partition){
		regs->rax=0;
		return;
	}
	user_partition_t* out=(void*)(regs->rsi);
	out->flags=partition->flags;
	out->type=partition->partition_config.type;
	out->index=partition->partition_config.index;
	out->first_block_index=partition->partition_config.first_block_index;
	out->last_block_index=partition->partition_config.last_block_index;
	_copy_string(out->name,partition->name,48);
	out->drive_index=partition->drive->index;
	regs->rax=1;
}
