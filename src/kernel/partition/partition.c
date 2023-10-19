#include <kernel/drive/drive.h>
#include <kernel/format/format.h>
#include <kernel/fs/fs.h>
#include <kernel/handle/handle.h>
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
#include <kernel/vfs/allocator.h>
#include <kernel/vfs/vfs.h>
#define KERNEL_LOG_NAME "partition"



PMM_DECLARE_COUNTER(OMM_PARTITION);



static omm_allocator_t _partition_allocator=OMM_ALLOCATOR_INIT_STRUCT("partition",sizeof(partition2_t),8,4,PMM_COUNTER_OMM_PARTITION);



HANDLE_DECLARE_TYPE(PARTITION,{
	drive2_t* partition=handle->object;
	WARN("Delete partition: %s",partition->name);
	omm_dealloc(&_partition_allocator,partition);
});



typedef struct __attribute__((packed)) _ISO9660_VOLUME_DESCRIPTOR{
	u8 type;
	u8 identifier[5];
	u8 version;
	struct __attribute__((packed)){
		u8 _padding[33];
		char volume_name[32];
		u8 _padding2[8];
		u32 volume_size;
		u8 _padding3[74];
		u32 directory_lba;
		u8 _padding4[4];
		u32 directory_data_length;
	} primary_volume_descriptor;
} iso9660_volume_descriptor_t;



void partition_init(void){
	partition_type_t partition_type_index=PARTITION_TYPE_UNKNOWN;
	for (const partition_descriptor_t*const* descriptor=(void*)kernel_section_partition_start();(u64)descriptor<kernel_section_partition_end();descriptor++){
		if (*descriptor){
			partition_type_index++;
			*((*descriptor)->var)=partition_type_index;
		}
	}
}



void partition_load_from_drive(drive2_t* drive){
	LOG("Loading partitions from drive '%s'...",drive->model_number);
	for (const partition_descriptor_t*const* descriptor=(void*)kernel_section_partition_start();(u64)descriptor<kernel_section_partition_end();descriptor++){
		if (*descriptor&&(*descriptor)->load_callback(drive)){
			drive->partition_type=*((*descriptor)->var);
			INFO("Detected drive partitioning as '%s'",(*descriptor)->name);
			return;
		}
	}
	WARN("Unable to detect partition type of drive '%s'",drive->model_number);
}



partition2_t* partition_create(drive2_t* drive,const char* name,u64 start_lba,u64 end_lba){
	LOG("Creating partition '%s' on drive '%s'...",name,drive->model_number);
	partition2_t* out=omm_alloc(&_partition_allocator);
	handle_new(out,HANDLE_TYPE_PARTITION,&(out->handle));
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
