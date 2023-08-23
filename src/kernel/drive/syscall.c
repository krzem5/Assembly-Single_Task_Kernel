#include <kernel/drive/drive.h>
#include <kernel/drive/drive_list.h>
#include <kernel/fs/kfs.h>
#include <kernel/memory/vmm.h>
#include <kernel/partition/partition.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/util.h>



#define USER_DRIVE_FLAG_PRESENT 1
#define USER_DRIVE_FLAG_BOOT 2



typedef struct _USER_DRIVE{
	u8 flags;
	u8 type;
	u8 index;
	char name[16];
	char serial_number[32];
	char model_number[64];
	u64 block_count;
	u64 block_size;
} user_drive_t;



void syscall_drive_list_length(syscall_registers_t* regs){
	regs->rax=drive_count;
}



void syscall_drive_list_get(syscall_registers_t* regs){
	if (regs->rdi>=drive_count||regs->rdx!=sizeof(user_drive_t)){
		regs->rax=-1;
		return;
	}
	u64 address=syscall_sanatize_user_memory(regs->rsi,regs->rdx);
	if (!address){
		regs->rax=-1;
		return;
	}
	const drive_t* drive=drive_data+regs->rdi;
	user_drive_t* user_drive=(void*)address;
	user_drive->flags=USER_DRIVE_FLAG_PRESENT|((drive->flags&DRIVE_FLAG_BOOT)?USER_DRIVE_FLAG_BOOT:0);
	user_drive->type=drive->type;
	user_drive->index=regs->rdi;
	memcpy(user_drive->name,drive->name,16);
	memcpy(user_drive->serial_number,drive->serial_number,32);
	memcpy(user_drive->model_number,drive->model_number,64);
	user_drive->block_count=drive->block_count;
	user_drive->block_size=drive->block_size;
	regs->rax=0;
}



void syscall_drive_format(syscall_registers_t* regs){
	if (regs->rdi>=drive_count){
		regs->rax=0;
		return;
	}
	u64 address=0;
	if (regs->rdx){
		address=syscall_sanatize_user_memory(regs->rsi,regs->rdx);
		if (!address){
			regs->rax=0;
			return;
		}
	}
	regs->rax=kfs_format_drive(drive_data+regs->rdi,(void*)address,regs->rdx);
}



void syscall_drive_stats(syscall_registers_t* regs){
	if (regs->rdi>=drive_count||regs->rdx!=sizeof(drive_stats_t)){
		regs->rax=0;
		return;
	}
	u64 address=syscall_sanatize_user_memory(regs->rsi,regs->rdx);
	if (!address){
		regs->rax=0;
		return;
	}
	partition_flush_cache();
	*((drive_stats_t*)address)=*((drive_data+regs->rdi)->stats);
	regs->rax=1;
}
