#include <kernel/memory/vmm.h>
#include <kernel/partition/partition.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/util.h>



#define USER_PARTITION_FLAG_PRESENT 1
#define USER_PARTITION_FLAG_BOOT 2
#define USER_PARTITION_FLAG_HALF_INSTALLED 4
#define USER_PARTITION_FLAG_PREVIOUS_BOOT 8



typedef struct _USER_PARTITION{
	u8 flags;
	u8 type;
	u8 index;
	u64 first_block_index;
	u64 last_block_index;
	char name[16];
	u32 drive_index;
} user_partition_t;



void syscall_partition_count(syscall_registers_t* regs){
	regs->rax=partition_count;
}



void syscall_partition_get(syscall_registers_t* regs){
	if (regs->rdi>=partition_count||regs->rdx!=sizeof(user_partition_t)){
		regs->rax=-1;
		return;
	}
	u64 address=syscall_sanatize_user_memory(regs->rsi,regs->rdx);
	if (!address){
		regs->rax=-1;
		return;
	}
	const partition_t* partition=partition_data2;
	for (u64 i=0;i<regs->rdi;i++){
		partition=partition->next;
		if (!partition){
			regs->rax=-1;
			return;
		}
	}
	user_partition_t* user_partition=(void*)address;
	user_partition->flags=USER_PARTITION_FLAG_PRESENT|((partition->flags&PARTITION_FLAG_BOOT)?USER_PARTITION_FLAG_BOOT:0)|((partition->flags&PARTITION_FLAG_HALF_INSTALLED)?USER_PARTITION_FLAG_HALF_INSTALLED:0)|((partition->flags&PARTITION_FLAG_PREVIOUS_BOOT)?USER_PARTITION_FLAG_PREVIOUS_BOOT:0);
	user_partition->type=partition->partition_config.type;
	user_partition->index=partition->partition_config.index;
	user_partition->first_block_index=partition->partition_config.first_block_index;
	user_partition->last_block_index=partition->partition_config.last_block_index;
	memcpy(user_partition->name,partition->name,16);
	user_partition->drive_index=partition->drive->index;
	regs->rax=0;
}
