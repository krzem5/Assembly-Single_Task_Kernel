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



void syscall_partition_get(syscall_registers_t* regs){
	if (regs->rdx!=sizeof(user_partition_t)||!syscall_sanatize_user_memory(regs->rsi,regs->rdx)){
		regs->rax=0;
		return;
	}
	regs->rax=0;
}
