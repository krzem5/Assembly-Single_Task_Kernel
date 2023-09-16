#include <kernel/drive/drive.h>
#include <kernel/fs/kfs.h>
#include <kernel/memory/vmm.h>
#include <kernel/partition/partition.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/util.h>



static drive_t* _get_drive(u64 index){
	for (drive_t* drive=drive_data;drive;drive=drive->next){
		if (drive->index==index){
			return drive;
		}
	}
	return NULL;
}



void syscall_drive_format(syscall_registers_t* regs){
	if (regs->rdx&&!syscall_sanatize_user_memory(regs->rsi,regs->rdx)){
		regs->rax=0;
		return;
	}
	const drive_t* drive=_get_drive(regs->rdi);
	if (!drive){
		regs->rax=0;
		return;
	}
	regs->rax=kfs_format_drive(drive,(void*)(regs->rsi),regs->rdx);
}



void syscall_drive_stats(syscall_registers_t* regs){
	if (regs->rdx!=sizeof(drive_stats_t)||!syscall_sanatize_user_memory(regs->rsi,regs->rdx)){
		regs->rax=0;
		return;
	}
	const drive_t* drive=_get_drive(regs->rdi);
	if (!drive){
		regs->rax=0;
		return;
	}
	partition_flush_cache();
	*((drive_stats_t*)(regs->rsi))=*(drive->stats);
	regs->rax=1;
}
