#include <kernel/drive/drive.h>
#include <kernel/fs/partition.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>



#define MAX_DRIVE_COUNT 32



static const char* _drive_type_names[]={
	[DRIVE_TYPE_AHCI]="AHCI",
	[DRIVE_TYPE_ATA]="ATA",
	[DRIVE_TYPE_ATAPI]="ATAPI",
	[DRIVE_TYPE_NVME]="NVME",
};



static drive_t* _drives;
static u32 _drive_count;



void drive_list_init(void){
	LOG("Initializing drive list...");
	_drives=VMM_TRANSLATE_ADDRESS(pmm_alloc(pmm_align_up_address(MAX_DRIVE_COUNT*sizeof(drive_t))));
	_drive_count=0;
}



void drive_list_add_drive(const drive_t* drive){
	LOG("Installing drive '%s/%s' as '%s'",_drive_type_names[drive->type],drive->model_number,drive->name);
	if (_drive_count>=MAX_DRIVE_COUNT){
		ERROR("Too many drives");
		return;
	}
	_drives[_drive_count]=*drive;
	(_drives+_drive_count)->flags=0;
	(_drives+_drive_count)->index=_drive_count;
	(_drives+_drive_count)->index=_drive_count;
	(_drives+_drive_count)->block_size_shift=__builtin_ctzll(drive->block_size);
	_drive_count++;
	INFO("Drive serial number: '%s', Drive size: %v (%lu * %lu)",drive->serial_number,drive->block_count*drive->block_size,drive->block_count,drive->block_size);
	if (drive->block_size>>((_drives+_drive_count-1)->block_size_shift+1)){
		WARN("Drive block size is not a power of 2");
	}
}



void drive_list_load_partitions(void){
	LOG("Loading drive partitions...");
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
