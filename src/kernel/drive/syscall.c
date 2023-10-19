#include <kernel/drive/drive.h>
#include <kernel/memory/vmm.h>
#include <kernel/partition/partition.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/util.h>



typedef struct _USER_DRIVE{
	u8 flags;
	u8 type;
	u8 index;
	char name[48];
	char serial_number[48];
	char model_number[48];
	u64 block_count;
	u64 block_size;
} user_drive_t;



void syscall_drive_get(syscall_registers_t* regs){
	if (regs->rdx!=sizeof(user_drive_t)||!syscall_sanatize_user_memory(regs->rsi,regs->rdx)){
		regs->rax=0;
		return;
	}
	regs->rax=0;
}
