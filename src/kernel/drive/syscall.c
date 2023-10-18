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



static void _copy_string(char* dst,const char* src,u8 maxlength){
	u8 i=0;
	for (;i<maxlength-1&&src[i];i++){
		dst[i]=src[i];
	}
	dst[i]=0;
}



static drive_t* _get_drive(u64 index){
	for (drive_t* drive=drive_data;drive;drive=drive->next){
		if (drive->index==index){
			return drive;
		}
	}
	return NULL;
}



void syscall_drive_get(syscall_registers_t* regs){
	if (regs->rdx!=sizeof(user_drive_t)||!syscall_sanatize_user_memory(regs->rsi,regs->rdx)){
		regs->rax=0;
		return;
	}
	const drive_t* drive=_get_drive(regs->rdi);
	if (!drive){
		regs->rax=0;
		return;
	}
	user_drive_t* out=(void*)(regs->rsi);
	out->flags=drive->flags;
	out->type=drive->type;
	out->index=drive->index;
	_copy_string(out->name,drive->name,48);
	_copy_string(out->serial_number,drive->serial_number,48);
	_copy_string(out->model_number,drive->model_number,48);
	out->block_count=drive->block_count;
	out->block_size=drive->block_size;
	regs->rax=1;
}



void syscall_drive_format(syscall_registers_t* regs){
	const drive_t* drive=_get_drive(regs->rdi);
	if (!drive){
		regs->rax=0;
		return;
	}
	regs->rax=0;
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
	*((drive_stats_t*)(regs->rsi))=*(drive->stats);
	regs->rax=1;
}
