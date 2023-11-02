#include <kernel/drive/drive.h>
#include <kernel/isr/isr.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/util.h>



#define DRIVE_DATA_NAME_LENGTH 64
#define DRIVE_DATA_SERIAL_NUMBER_LENGTH 64
#define DRIVE_DATA_MODEL_NUMBER_LENGTH 64
#define DRIVE_DATA_TYPE_LENGTH 64



typedef struct _USER_DRIVE_DATA{
	char name[DRIVE_DATA_NAME_LENGTH];
	char serial_number[DRIVE_DATA_SERIAL_NUMBER_LENGTH];
	char model_number[DRIVE_DATA_MODEL_NUMBER_LENGTH];
	char type[DRIVE_DATA_TYPE_LENGTH];
	u64 block_count;
	u64 block_size;
} user_drive_data_t;



void syscall_drive_get_data(isr_state_t* regs){
	if (regs->rdx!=sizeof(user_drive_data_t)||!syscall_sanatize_user_memory(regs->rsi,regs->rdx)){
		regs->rax=0;
		return;
	}
	handle_t* handle=handle_lookup_and_acquire(regs->rdi,HANDLE_TYPE_DRIVE);
	if (!handle){
		regs->rax=0;
		return;
	}
	user_drive_data_t* out=(void*)(regs->rsi);
	drive_t* drive=handle->object;
	strcpy(out->name,drive->name,DRIVE_DATA_NAME_LENGTH);
	strcpy(out->serial_number,drive->serial_number,DRIVE_DATA_SERIAL_NUMBER_LENGTH);
	strcpy(out->model_number,drive->model_number,DRIVE_DATA_MODEL_NUMBER_LENGTH);
	strcpy(out->type,drive->type->name,DRIVE_DATA_TYPE_LENGTH);
	out->block_count=drive->block_count;
	out->block_size=drive->block_size;
	regs->rax=1;
}
