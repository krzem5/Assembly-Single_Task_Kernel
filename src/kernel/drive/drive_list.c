#include <kernel/drive/drive.h>
#include <kernel/fs/partition.h>
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



static drive_t* KERNEL_CORE_DATA _drives;
static drive_stats_t* KERNEL_CORE_DATA _drive_stats;
static u32 KERNEL_CORE_DATA _drive_count;



void KERNEL_CORE_CODE drive_list_init(void){
	LOG_CORE("Initializing drive list...");
	_drives=VMM_TRANSLATE_ADDRESS(pmm_alloc(pmm_align_up_address(MAX_DRIVE_COUNT*sizeof(drive_t))>>PAGE_SIZE_SHIFT,PMM_COUNTER_DRIVE_LIST));
	_drive_stats=VMM_TRANSLATE_ADDRESS(pmm_alloc(pmm_align_up_address(MAX_DRIVE_COUNT*sizeof(drive_stats_t))>>PAGE_SIZE_SHIFT,PMM_COUNTER_DRIVE_LIST));
	_drive_count=0;
}



void KERNEL_CORE_CODE drive_list_add_drive(const drive_t* drive){
	LOG_CORE("Installing drive '%s/%s' as '%s'",_drive_type_names[drive->type],drive->model_number,drive->name);
	if (_drive_count>=MAX_DRIVE_COUNT){
		ERROR_CORE("Too many drives");
		return;
	}
	_drives[_drive_count]=*drive;
	(_drives+_drive_count)->flags=0;
	(_drives+_drive_count)->index=_drive_count;
	(_drives+_drive_count)->index=_drive_count;
	(_drives+_drive_count)->block_size_shift=__builtin_ctzll(drive->block_size);
	(_drives+_drive_count)->stats=_drive_stats+_drive_count;
	(_drives+_drive_count)->stats->root_block_count=0;
	(_drives+_drive_count)->stats->batc_block_count=0;
	(_drives+_drive_count)->stats->nda3_block_count=0;
	(_drives+_drive_count)->stats->nda2_block_count=0;
	(_drives+_drive_count)->stats->nda1_block_count=0;
	(_drives+_drive_count)->stats->nfda_block_count=0;
	(_drives+_drive_count)->stats->data_block_count=0;
	_drive_count++;
	INFO_CORE("Drive serial number: '%s', Drive size: %v (%lu * %lu)",drive->serial_number,drive->block_count*drive->block_size,drive->block_count,drive->block_size);
	if (drive->block_size>>((_drives+_drive_count-1)->block_size_shift+1)){
		WARN_CORE("Drive block size is not a power of 2");
	}
}



void KERNEL_CORE_CODE drive_list_load_partitions(void){
	LOG_CORE("Loading drive partitions...");
	for (u32 i=0;i<_drive_count;i++){
		fs_partition_load_from_drive(_drives+i);
	}
}



u32 drive_list_get_length(void){
	return _drive_count;
}



const drive_t* drive_list_get_drive(u32 index){
	return (index<_drive_count?_drives+index:NULL);
}



void KERNEL_CORE_CODE drive_list_set_boot_drive(u32 index){
	(_drives+index)->flags|=DRIVE_FLAG_BOOT;
}
