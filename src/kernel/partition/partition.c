#include <kernel/drive/drive.h>
#include <kernel/format/format.h>
#include <kernel/fs/fs.h>
#include <kernel/handle/handle.h>
#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/partition/partition.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "partition"



static pmm_counter_descriptor_t _partition_omm_pmm_counter=PMM_COUNTER_INIT_STRUCT("omm_partition");
static omm_allocator_t _partition_allocator=OMM_ALLOCATOR_INIT_STRUCT("partition",sizeof(partition_t),8,4,&_partition_omm_pmm_counter);



HANDLE_DECLARE_TYPE(PARTITION,{
	partition_t* partition=handle->object;
	WARN("Delete partition: %s",partition->name);
	if (partition->descriptor){
		handle_release(&(partition->descriptor->handle));
	}
	omm_dealloc(&_partition_allocator,partition);
});
HANDLE_DECLARE_TYPE(PARTITION_TABLE_DESCRIPTOR,{});



void partition_register_table_descriptor(partition_table_descriptor_t* descriptor){
	LOG("Registering partition table descriptor '%s'...",descriptor->name);
	handle_new(descriptor,HANDLE_TYPE_PARTITION_TABLE_DESCRIPTOR,&(descriptor->handle));
	HANDLE_FOREACH(HANDLE_TYPE_DRIVE){
		drive_t* drive=handle->object;
		if (drive->partition_descriptor){
			continue;
		}
		handle_acquire(&(descriptor->handle));
		drive->partition_descriptor=descriptor;
		if (descriptor->load_callback(drive)){
			INFO("Detected partitioning of drive '%s' as '%s'",drive->model_number,descriptor->name);
		}
		else{
			drive->partition_descriptor=NULL;
			handle_release(&(descriptor->handle));
		}
	}
}



void partition_unregister_table_descriptor(partition_table_descriptor_t* descriptor){
	LOG("Unregistering partition table descriptor '%s'...",descriptor->name);
	handle_destroy(&(descriptor->handle));
}



void partition_load_from_drive(drive_t* drive){
	LOG("Loading partitions from drive '%s'...",drive->model_number);
	HANDLE_FOREACH(HANDLE_TYPE_PARTITION_TABLE_DESCRIPTOR){
		partition_table_descriptor_t* descriptor=handle->object;
		handle_acquire(&(descriptor->handle));
		drive->partition_descriptor=descriptor;
		if (descriptor->load_callback(drive)){
			INFO("Detected drive partitioning as '%s'",descriptor->name);
			return;
		}
		handle_release(&(descriptor->handle));
	}
	drive->partition_descriptor=NULL;
	WARN("Unable to detect partition type of drive '%s'",drive->model_number);
}



partition_t* partition_create(drive_t* drive,const char* name,u64 start_lba,u64 end_lba){
	LOG("Creating partition '%s' on drive '%s'...",name,drive->model_number);
	handle_acquire(&(drive->partition_descriptor->handle));
	partition_t* out=omm_alloc(&_partition_allocator);
	handle_new(out,HANDLE_TYPE_PARTITION,&(out->handle));
	out->descriptor=drive->partition_descriptor;
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
