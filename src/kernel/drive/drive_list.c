#include <kernel/drive/drive.h>
#include <kernel/partition/partition.h>
#include <kernel/log/log.h>
#include <kernel/memory/kmm.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "drive_list"



static KERNEL_CORE_RDATA const char _drive_type_name_ahci[]="AHCI";
static KERNEL_CORE_RDATA const char _drive_type_name_ata[]="ATA";
static KERNEL_CORE_RDATA const char _drive_type_name_atapi[]="ATAPI";
static KERNEL_CORE_RDATA const char _drive_type_name_nvme[]="NVME";



static const char*const KERNEL_CORE_RDATA _drive_type_names[]={
	[DRIVE_TYPE_AHCI]=_drive_type_name_ahci,
	[DRIVE_TYPE_ATA]=_drive_type_name_ata,
	[DRIVE_TYPE_ATAPI]=_drive_type_name_atapi,
	[DRIVE_TYPE_NVME]=_drive_type_name_nvme,
};



static u32 KERNEL_CORE_BSS _drive_count;

drive_t* KERNEL_CORE_BSS drive_data;



void KERNEL_CORE_CODE drive_list_add_drive(const drive_t* drive){
	LOG_CORE("Installing drive '%s/%s' as '%s'",_drive_type_names[drive->type],drive->model_number,drive->name);
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



void KERNEL_CORE_CODE drive_list_load_partitions(void){
	LOG_CORE("Loading drive partitions...");
	for (drive_t* drive=drive_data;drive;drive=drive->next){
		partition_load_from_drive(drive);
	}
}
