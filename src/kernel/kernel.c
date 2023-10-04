#include <kernel/drive/drive.h>
#include <kernel/format/format.h>
#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/partition/partition.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/vfs/vfs.h>
#define KERNEL_LOG_NAME "kernel"



#define ADJUST_SECTION_FLAGS(name,flags) \
	INFO("Marking region %p - %p [%v] as %s+%s",kernel_section_##name##_start(),kernel_section_##name##_end(),pmm_align_up_address(kernel_section_##name##_end()-kernel_section_##name##_start()),(((flags)&VMM_PAGE_FLAG_READWRITE)?"RW":"RD"),(((flags)&VMM_PAGE_FLAG_NOEXECUTE)?"NX":"EX")); \
	vmm_adjust_flags(&vmm_kernel_pagemap,kernel_section_##name##_start(),(flags),VMM_PAGE_FLAG_READWRITE,pmm_align_up_address(kernel_section_##name##_end()-kernel_section_##name##_start())>>PAGE_SIZE_SHIFT)



static KERNEL_CORE_RDATA const char _kernel_memory_unusable[]=" (Unusable)";
static KERNEL_CORE_RDATA const char _kernel_memory_normal[]="";

static KERNEL_CORE_RDATA const char _kernel_file_path_format_template[]="%s:/kernel/kernel.bin";



void KERNEL_CORE_CODE KERNEL_NOCOVERAGE kernel_init(void){
	LOG_CORE("Loading kernel data...");
	INFO_CORE("Version: %lx",kernel_get_version());
	INFO_CORE("Core kernel range: %p - %p (%v)",kernel_section_core_start()-kernel_get_offset(),kernel_section_core_end()-kernel_get_offset(),kernel_section_core_end()-kernel_section_core_start());
	INFO_CORE("BSS kernel range: %p - %p (%v)",kernel_section_bss_start()-kernel_get_offset(),kernel_section_bss_end()-kernel_get_offset(),kernel_section_bss_end()-kernel_section_bss_start());
	INFO_CORE("Full kernel range: %p - %p (%v)",kernel_section_address_range_start()-kernel_get_offset(),kernel_section_address_range_end()-kernel_get_offset(),kernel_section_address_range_end()-kernel_section_address_range_start());
	INFO_CORE("Mmap Data:");
	u64 total=0;
	for (u16 i=0;i<KERNEL_DATA->mmap_size;i++){
		INFO_CORE("  %p - %p%s",(KERNEL_DATA->mmap+i)->base,(KERNEL_DATA->mmap+i)->base+(KERNEL_DATA->mmap+i)->length,((KERNEL_DATA->mmap+i)->type==1?_kernel_memory_normal:_kernel_memory_unusable));
		if ((KERNEL_DATA->mmap+i)->type==1){
			total+=(KERNEL_DATA->mmap+i)->length;
		}
	}
	INFO_CORE("Total: %v",total);
	LOG_CORE("Clearing .bss section (%v)...",kernel_section_bss_end()-kernel_section_bss_start());
	for (u64* bss=(u64*)kernel_section_bss_start();bss<(u64*)kernel_section_bss_end();bss++){
		*bss=0;
	}
}



void KERNEL_CORE_CODE kernel_load(void){
	LOG_CORE("Searching drives for the boot drive...");
	u8 buffer[4096];
	const drive_t* boot_drive=NULL;
	const drive_t* prev_boot_drive=NULL;
	for (drive_t* drive=drive_data;drive;drive=drive->next){
		memset(buffer,0,4096);
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
		for (partition_t* partition=partition_data;partition;partition=partition->next){
			if (partition->drive==prev_boot_drive&&partition->partition_config.type==PARTITION_CONFIG_TYPE_KFS){
				partition->flags|=PARTITION_FLAG_PREVIOUS_BOOT;
				break;
			}
		}
	}
	char path[64];
_check_every_drive:
	for (partition_t* partition=partition_data;partition;partition=partition->next){
		if (boot_drive&&partition->drive!=boot_drive){
			continue;
		}
		format_string(path,64,_kernel_file_path_format_template,partition->name);
		INFO_CORE("Trying to load the kernel from '%s'...",path);
		vfs_node_t* kernel=vfs_get_by_path(NULL,path,0);
		if (!kernel){
			if (boot_drive&&partition->partition_config.type==PARTITION_CONFIG_TYPE_KFS){
				partition->flags|=PARTITION_FLAG_HALF_INSTALLED;
			}
			continue;
		}
		INFO_CORE("File found, reading header...");
		if (vfs_read(kernel,0,buffer,partition->drive->block_size)!=partition->drive->block_size){
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
		LOG_CORE("Found boot drive: %s (%s)",partition->name,partition->drive->model_number);
		partition->flags|=PARTITION_FLAG_BOOT;
		partition_boot=partition;
		((drive_t*)(partition->drive))->flags|=DRIVE_FLAG_BOOT;
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
	vfs_node_t* kernel_file=vfs_get_by_path(NULL,path,0);
	if (!kernel_file){
		goto _error;
	}
	u64 kernel_size=kernel_section_kernel_end()-kernel_section_core_end();
	void* address=(void*)kernel_section_core_end();
	INFO_CORE("Reading %v from '/kernel.bin' to address %p...",kernel_size,address);
	u64 rd=vfs_read(kernel_file,0,address,kernel_size);
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
	panic("Unable to load kernel");
}



void kernel_adjust_memory_flags(void){
	LOG("Adjusting memory flags...");
	ADJUST_SECTION_FLAGS(core_ex,0);
	ADJUST_SECTION_FLAGS(core_nx,VMM_PAGE_FLAG_NOEXECUTE);
	ADJUST_SECTION_FLAGS(core_rw,VMM_PAGE_FLAG_NOEXECUTE|VMM_PAGE_FLAG_READWRITE);
	ADJUST_SECTION_FLAGS(kernel_ex,0);
	ADJUST_SECTION_FLAGS(kernel_nx,VMM_PAGE_FLAG_NOEXECUTE);
	ADJUST_SECTION_FLAGS(kernel_rw,VMM_PAGE_FLAG_NOEXECUTE|VMM_PAGE_FLAG_READWRITE);
	ADJUST_SECTION_FLAGS(bss,VMM_PAGE_FLAG_NOEXECUTE|VMM_PAGE_FLAG_READWRITE);
}



const char* kernel_lookup_symbol(u64 address,u64* offset){
	if (address<kernel_section_address_range_start()||address>=kernel_section_address_range_end()){
		if (offset){
			*offset=0;
		}
		return NULL;
	}
	u32 index=0xffffffff;
	for (u32 i=0;kernel_symbols[i];i+=2){
		if (address>=kernel_symbols[i]&&(index==0xffffffff||kernel_symbols[i]>kernel_symbols[index])){
			index=i;
		}
	}
	if (offset){
		*offset=address-kernel_symbols[index];
	}
	return (void*)(kernel_symbols[index+1]);
}
