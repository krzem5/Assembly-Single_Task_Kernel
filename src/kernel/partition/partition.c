#include <kernel/drive/drive.h>
#include <kernel/format/format.h>
#include <kernel/fs/fs.h>
#include <kernel/handle/handle.h>
#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/memory/kmm.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/partition/partition.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "partition"



PMM_DECLARE_COUNTER(OMM_PARTITION);



static omm_allocator_t _partition_allocator=OMM_ALLOCATOR_INIT_STRUCT("partition",sizeof(partition_t),8,4,PMM_COUNTER_OMM_PARTITION);



HANDLE_DECLARE_TYPE(PARTITION,{
	drive_t* partition=handle->object;
	WARN("Delete partition: %s",partition->name);
	omm_dealloc(&_partition_allocator,partition);
});



void partition_init(void){
	partition_type_t partition_type_index=PARTITION_TYPE_UNKNOWN;
	for (const partition_descriptor_t*const* descriptor=(void*)kernel_section_partition_start();(u64)descriptor<kernel_section_partition_end();descriptor++){
		partition_type_index++;
		*((*descriptor)->var)=partition_type_index;
	}
}



void partition_load_from_drive(drive_t* drive){
	LOG("Loading partitions from drive '%s'...",drive->model_number);
	for (const partition_descriptor_t*const* descriptor=(void*)kernel_section_partition_start();(u64)descriptor<kernel_section_partition_end();descriptor++){
		drive->partition_type=*((*descriptor)->var);
		if ((*descriptor)->load_callback(drive)){
			INFO("Detected drive partitioning as '%s'",(*descriptor)->name);
			return;
		}
	}
	drive->partition_type=PARTITION_TYPE_UNKNOWN;
	WARN("Unable to detect partition type of drive '%s'",drive->model_number);
}



partition_t* partition_create(drive_t* drive,const char* name,u64 start_lba,u64 end_lba){
	LOG("Creating partition '%s' on drive '%s'...",name,drive->model_number);
	partition_t* out=omm_alloc(&_partition_allocator);
	handle_new(out,HANDLE_TYPE_PARTITION,&(out->handle));
	out->partition_descriptor=((const partition_descriptor_t*const*)kernel_section_partition_start())[drive->partition_type-PARTITION_TYPE_UNKNOWN-1];
	out->drive=drive;
	memcpy(out->name,name,32);
	out->start_lba=start_lba;
	out->end_lba=end_lba;
	out->fs=fs_load(out);
	if (!out->fs){
		WARN("No filesystem detected on partition '%s/%s'",drive->name,name);
	}
	return out;
}
