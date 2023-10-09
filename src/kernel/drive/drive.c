#include <kernel/drive/drive.h>
#include <kernel/partition/partition.h>
#include <kernel/log/log.h>
#include <kernel/memory/kmm.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "drive"



static const char* _drive_type_names[]={
	[DRIVE_TYPE_AHCI]="AHCI",
	[DRIVE_TYPE_ATA]="ATA",
	[DRIVE_TYPE_ATAPI]="ATAPI",
	[DRIVE_TYPE_NVME]="NVME",
};



static u32 KERNEL_BSS _drive_count;

drive_t* KERNEL_BSS drive_data;



void drive_add(const drive_t* drive){
	LOG_CORE("Creating drive '%s' as '%s/%s'...",drive->name,_drive_type_names[drive->type],drive->model_number);
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
	INFO_CORE("Drive serial number: '%s', Drive size: %v (%lu * %lu)",drive->serial_number,drive->block_count*drive->block_size,drive->block_count,drive->block_size);
	if (drive->block_size>>(new_drive->block_size_shift+1)){
		WARN_CORE("Drive block size is not a power of 2");
	}
}
