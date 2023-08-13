#include <kernel/drive/drive.h>
#include <kernel/partition/partition.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "drive_list"



#define MAX_DRIVE_COUNT 32



static KERNEL_CORE_RDATA const char _drive_type_name_ahci[]="AHCI";
static KERNEL_CORE_RDATA const char _drive_type_name_ata[]="ATA";
static KERNEL_CORE_RDATA const char _drive_type_name_atapi[]="ATAPI";
static KERNEL_CORE_RDATA const char _drive_type_name_nvme[]="NVME";



static const char* KERNEL_CORE_DATA _drive_type_names[]={
	[DRIVE_TYPE_AHCI]=_drive_type_name_ahci,
	[DRIVE_TYPE_ATA]=_drive_type_name_ata,
	[DRIVE_TYPE_ATAPI]=_drive_type_name_atapi,
	[DRIVE_TYPE_NVME]=_drive_type_name_nvme,
};



static drive_stats_t KERNEL_CORE_BSS _drive_stats[MAX_DRIVE_COUNT];

drive_t KERNEL_CORE_BSS drive_data[MAX_DRIVE_COUNT];
u32 KERNEL_CORE_BSS drive_count;



void KERNEL_CORE_CODE drive_list_add_drive(const drive_t* drive){
	LOG_CORE("Installing drive '%s/%s' as '%s'",_drive_type_names[drive->type],drive->model_number,drive->name);
	if (drive_count>=MAX_DRIVE_COUNT){
		ERROR_CORE("Too many drives");
		return;
	}
	drive_data[drive_count]=*drive;
	(drive_data+drive_count)->flags=0;
	(drive_data+drive_count)->index=drive_count;
	(drive_data+drive_count)->index=drive_count;
	(drive_data+drive_count)->block_size_shift=__builtin_ctzll(drive->block_size);
	(drive_data+drive_count)->stats=_drive_stats+drive_count;
	(drive_data+drive_count)->stats->root_block_count=0;
	(drive_data+drive_count)->stats->batc_block_count=0;
	(drive_data+drive_count)->stats->nda3_block_count=0;
	(drive_data+drive_count)->stats->nda2_block_count=0;
	(drive_data+drive_count)->stats->nda1_block_count=0;
	(drive_data+drive_count)->stats->nfda_block_count=0;
	(drive_data+drive_count)->stats->data_block_count=0;
	drive_count++;
	INFO_CORE("Drive serial number: '%s', Drive size: %v (%lu * %lu)",drive->serial_number,drive->block_count*drive->block_size,drive->block_count,drive->block_size);
	if (drive->block_size>>((drive_data+drive_count-1)->block_size_shift+1)){
		WARN_CORE("Drive block size is not a power of 2");
	}
}



void KERNEL_CORE_CODE drive_list_load_partitions(void){
	LOG_CORE("Loading drive partitions...");
	for (u32 i=0;i<drive_count;i++){
		partition_load_from_drive(drive_data+i);
	}
}
