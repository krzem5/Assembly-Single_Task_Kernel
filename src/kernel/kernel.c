#include <kernel/drive/drive_list.h>
#include <kernel/fs/fs.h>
#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "kernel"



static KERNEL_CORE_RDATA const char _kernel_memory_unusable[]=" (Unusable)";
static KERNEL_CORE_RDATA const char _kernel_memory_normal[]="";
static KERNEL_CORE_RDATA const char _kernel_memory_acpi[]=" (ACPI tables)";

static KERNEL_CORE_RDATA const char _kernel_file_path[]="/kernel.bin";



const kernel_data_t* KERNEL_CORE_CODE kernel_init(void){
	LOG_CORE("Loading kernel data...");
	const kernel_data_t* kernel_data=(const kernel_data_t*)0xffffffffc0007000;
	INFO_CORE("Version: %lx",kernel_get_version());
	INFO_CORE("Core kernel range: %p - %p",kernel_get_start(),kernel_get_core_end());
	INFO_CORE("Full kernel range: %p - %p",kernel_get_start(),kernel_get_end());
	INFO_CORE("Mmap Data:");
	u64 total=0;
	for (u16 i=0;i<kernel_data->mmap_size;i++){
		const char* type=_kernel_memory_unusable;
		switch ((kernel_data->mmap+i)->type){
			case 1:
				type=_kernel_memory_normal;
				break;
			case 3:
				type=_kernel_memory_acpi;
				break;
		}
		INFO_CORE("  %p - %p%s",(kernel_data->mmap+i)->base,(kernel_data->mmap+i)->base+(kernel_data->mmap+i)->length,type);
		if ((kernel_data->mmap+i)->type==1){
			total+=(kernel_data->mmap+i)->length;
		}
	}
	INFO_CORE("Total: %v",total);
	return kernel_data;
}



void KERNEL_CORE_CODE kernel_load(void){
	LOG_CORE("Searching drives for the boot drive...");
	u8 buffer[4096];
	const drive_t* boot_drive=NULL;
	const drive_t* prev_boot_drive=NULL;
	for (u8 i=0;1;i++){
		const drive_t* drive=drive_list_get_drive(i);
		if (!drive){
			break;
		}
		if ((drive->type==DRIVE_TYPE_AHCI||drive->type==DRIVE_TYPE_NVME)&&drive->block_size<=4096&&drive->read_write(drive->extra_data,0,buffer,1)==1){
			u64 drive_version=*((const u64*)(buffer+64));
			if (drive_version==kernel_get_version()){
				boot_drive=drive;
			}
			else if (drive_version&&drive_version<kernel_get_version()){
				prev_boot_drive=drive;
			}
		}
	}
	if (boot_drive){
		LOG_CORE("Found the boot drive at '%s'",boot_drive->name);
	}
	else{
		LOG_CORE("Searching partitions for the boot drive...");
	}
	if (prev_boot_drive){
		LOG_CORE("Searching for previous boot drive partition...");
		for (u8 fs_index=0;1;fs_index++){
			const fs_file_system_t* fs=fs_get_file_system(fs_index);
			if (!fs){
				break;
			}
			if (fs->drive==prev_boot_drive&&fs->partition_config.type==FS_PARTITION_TYPE_KFS){
				fs_set_previous_boot_file_system(fs_index);
				break;
			}
		}
	}
	char path[64];
_check_every_drive:
	for (u8 fs_index=0;1;fs_index++){
		const fs_file_system_t* fs=fs_get_file_system(fs_index);
		if (!fs){
			break;
		}
		if (boot_drive){
			if (fs->drive!=boot_drive){
				continue;
			}
		}
		u8 i=0;
		while (fs->name[i]){
			path[i]=fs->name[i];
			i++;
		}
		path[i]=':';
		i++;
		for (u8 j=0;_kernel_file_path[j];j++){
			path[i]=_kernel_file_path[j];
			i++;
		}
		path[i]=0;
		INFO_CORE("Trying to load the kernel from '%s'...",path);
		fs_node_t* kernel=fs_node_get_by_path(NULL,path,0);
		if (!kernel){
			if (boot_drive&&fs->partition_config.type==FS_PARTITION_TYPE_KFS){
				fs_set_half_installed_file_system(fs_index);
			}
			continue;
		}
		INFO_CORE("File found, reading header...");
		if (fs_node_read(kernel,0,buffer,fs->drive->block_size)!=fs->drive->block_size){
			WARN_CORE("Not a valid kernel file");
			continue;
		}
		INFO_CORE("Checking kernel version...");
		u64 version=*((u64*)buffer);
		if (version!=kernel_get_version()){
			WARN_CORE("Kernel file version mismach");
			INFO_CORE("Expected %lx, got %lx",kernel_get_version(),version);
			continue;
		}
		LOG_CORE("Found boot drive: %s (%s)",fs->name,fs->drive->model_number);
		fs_set_boot_file_system(fs_index);
		goto _load_kernel;
	}
	if (boot_drive){
		WARN_CORE("Unable to load the kernel from the boot drive, checking all drives...");
		boot_drive=NULL;
		goto _check_every_drive;
	}
	goto _error;
_load_kernel:
	LOG_CORE("Loading kernel...");
	INFO_CORE("Opening kernel file...");
	fs_node_t* kernel_file=fs_node_get_by_path(NULL,_kernel_file_path,0);
	if (!kernel_file){
		goto _error;
	}
	u64 kernel_size=kernel_get_end()-kernel_get_core_end();
	void* address=(void*)(kernel_get_core_end()+kernel_get_offset());
	INFO_CORE("Reading %v from '/kernel.bin' to address %p...",kernel_size,address);
	u64 rd=fs_node_read(kernel_file,0,address,kernel_size);
	if (rd!=kernel_size){
		goto _error;
	}
	u64 version=*((u64*)address);
	if (version!=kernel_get_version()){
		goto _error;
	}
	LOG_CORE("Kernel successfully loaded");
	return;
_error:
	ERROR_CORE("Unable to load kernel");
	for (;;);
}
