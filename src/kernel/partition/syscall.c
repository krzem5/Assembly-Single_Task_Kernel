#include <kernel/partition/partition.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/util.h>



#define PARTITION_DATA_NAME_LENGTH 64

#define PARTITION_DATA_PARTITION_TABLE_NAME_LENGTH 64



typedef struct _USER_PARTITION_DATA{
	char name[PARTITION_DATA_NAME_LENGTH];
	char partition_table_name[PARTITION_DATA_PARTITION_TABLE_NAME_LENGTH];
	u64 drive_handle;
	u64 start_lba;
	u64 end_lba;
	u64 fs_handle;
} user_partition_data_t;



void syscall_partition_get(syscall_registers_t* regs){
	regs->rax=0;
}



void syscall_partition_get_data(syscall_registers_t* regs){
	if (regs->rdx!=sizeof(user_partition_data_t)||!syscall_sanatize_user_memory(regs->rsi,regs->rdx)){
		return;
	}
	handle_t* handle=handle_lookup_and_acquire(regs->rdi,HANDLE_TYPE_PARTITION);
	if (!handle){
		regs->rax=0;
		return;
	}
	user_partition_data_t* out=(void*)(regs->rsi);
	partition_t* partition=handle->object;
	strcpy(out->name,partition->name,PARTITION_DATA_NAME_LENGTH);
	strcpy(out->partition_table_name,partition->partition_descriptor->name,PARTITION_DATA_PARTITION_TABLE_NAME_LENGTH);
	out->drive_handle=partition->drive->handle.rb_node.key;
	out->start_lba=partition->start_lba;
	out->end_lba=partition->end_lba;
	out->fs_handle=partition->fs->handle.rb_node.key;
	regs->rax=1;
}

