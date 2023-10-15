#include <kernel/drive/drive.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/memory/kmm.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/partition/partition.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "drive"



PMM_DECLARE_COUNTER(OMM_DRIVE);



static omm_allocator_t _drive_allocator=OMM_ALLOCATOR_INIT_STRUCT("drive",sizeof(drive2_t),8,4,PMM_COUNTER_OMM_DRIVE);



HANDLE_DECLARE_TYPE(DRIVE,{
	drive2_t* drive=handle->object;
	WARN("Delete drive: %s",drive->name);
	omm_dealloc(&_drive_allocator,drive);
});



static const char* _drive_type_names[]={
	[DRIVE_TYPE_AHCI]="AHCI",
	[DRIVE_TYPE_ATA]="ATA",
	[DRIVE_TYPE_ATAPI]="ATAPI",
	[DRIVE_TYPE_NVME]="NVME",
};



static u32 KERNEL_BSS _drive_count;

drive_t* KERNEL_BSS drive_data;



drive2_t* drive_create(const drive_config_t* config){
	LOG("Creating drive '%s' as '%s/%s'...",config->name,_drive_type_names[config->type],config->model_number);
	drive2_t* out=omm_alloc(&_drive_allocator);
	handle_new(out,HANDLE_TYPE_DRIVE,&(out->handle));
	out->type=config->type;
	out->flags=config->flags;
	out->block_size_shift=__builtin_ctzll(config->block_size);
	out->_next_partition_index=0;
	memcpy(out->name,config->name,DRIVE_NAME_LENGTH);
	memcpy(out->serial_number,config->serial_number,DRIVE_SERIAL_NUMBER_LENGTH);
	memcpy(out->model_number,config->model_number,DRIVE_MODEL_NUMBER_LENGTH);
	out->block_count=config->block_count;
	out->block_size=config->block_size;
	out->read_write=config->read_write;
	out->extra_data=config->extra_data;
	INFO("Drive serial number: '%s', Drive size: %v (%lu * %lu)",out->serial_number,out->block_count*out->block_size,out->block_count,out->block_size);
	if (out->block_size&(out->block_size-1)){
		WARN("Drive block size is not a power of 2");
	}
	partition_load_from_drive(out);
	return out;
}



void drive_add(const drive_t* drive){
	drive_t* new_drive=kmm_alloc(sizeof(drive_t));
	*new_drive=*drive;
	new_drive->next=drive_data;
	drive_data=new_drive;
	new_drive->flags=0;
	new_drive->index=_drive_count;
	new_drive->block_size_shift=__builtin_ctzll(drive->block_size);
	new_drive->stats=kmm_alloc(sizeof(drive_stats_t));
	new_drive->stats->root_block_count=0;
	new_drive->stats->batc_block_count=0;
	new_drive->stats->nda3_block_count=0;
	new_drive->stats->nda2_block_count=0;
	new_drive->stats->nda1_block_count=0;
	new_drive->stats->nfda_block_count=0;
	new_drive->stats->data_block_count=0;
	_drive_count++;
}
