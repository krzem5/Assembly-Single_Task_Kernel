#include <kernel/fs/fs.h>
#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>



static const char* _kernel_file_path=":/kernel.bin";

static kernel_data_t _kernel_data;



const kernel_data_t* kernel_init(void){
	LOG("Loading kernel data...");
	_kernel_data=*((const volatile kernel_data_t*)0xffffffffc0007000);
	INFO("Version: %lx",kernel_get_version());
	INFO("Low kernel range: %p - %p",kernel_get_start(),kernel_get_low_end());
	INFO("High kernel range: %p - %p",kernel_get_low_end(),kernel_get_end());
	INFO("Mmap Data:");
	u64 total=0;
	for (u16 i=0;i<_kernel_data.mmap_size;i++){
		const char* type=" (Unusable)";
		switch ((_kernel_data.mmap+i)->type){
			case 1:
				type="";
				break;
			case 3:
				type=" (ACPI tables)";
				break;
		}
		INFO("  %p - %p%s",(_kernel_data.mmap+i)->base,(_kernel_data.mmap+i)->base+(_kernel_data.mmap+i)->length,type);
		if ((_kernel_data.mmap+i)->type==1){
			total+=(_kernel_data.mmap+i)->length;
		}
	}
	INFO("Total: %v",total);
	return &_kernel_data;
}



void kernel_load(void){
	LOG("Searching partitions for the boot drive...");
	u8 buffer[4096];
	char path[64];
	for (u8 fs_index=0;1;fs_index++){
		const fs_file_system_t* fs=fs_get_file_system(fs_index);
		if (!fs){
			break;
		}
		u8 i=0;
		while (fs->name[i]){
			path[i]=fs->name[i];
			i++;
		}
		for (u8 j=0;_kernel_file_path[j];j++){
			path[i]=_kernel_file_path[j];
			i++;
		}
		path[i]=0;
		INFO("Trying to load the kernel from '%s'...",path);
		fs_node_t* kernel=fs_get_node(NULL,path,0);
		if (!kernel){
			continue;
		}
		INFO("File found, reading header...");
		if (fs_read(kernel,0,buffer,fs->drive->block_size)!=fs->drive->block_size){
			WARN("Not a valid kernel file");
			continue;
		}
		INFO("Checking kernel version...");
		u64 version=*((u64*)buffer);
		if (version!=kernel_get_version()){
			WARN("Kernel file version mismach");
			INFO("Expected %lx, got %lx",kernel_get_version(),version);
			continue;
		}
		LOG("Found boot drive: %s (%s)",fs->name,fs->drive->model_number);
		fs_set_boot_file_system(fs_index);
		goto _load_kernel;
	}
	goto _error;
_load_kernel:
	LOG("Loading kernel...");
	INFO("Opening kernel file...");
	fs_node_t* kernel_file=fs_get_node(NULL,"/kernel.bin",0);
	if (!kernel_file){
		goto _error;
	}
	u64 kernel_size=kernel_get_end()-kernel_get_low_end();
	void* address=(void*)(kernel_get_low_end()+kernel_get_offset());
	INFO("Reading %v from '/kernel.bin' to address %p...",kernel_size,address);
	if (fs_read(kernel_file,0,address,kernel_size)!=kernel_size){
		goto _error;
	}
	u64 version=*((u64*)address);
	if (version!=kernel_get_version()){
		goto _error;
	}
	LOG("Kernel successfully loaded");
	return;
_error:
	ERROR("Unable to load kernel");
	for (;;);
}
