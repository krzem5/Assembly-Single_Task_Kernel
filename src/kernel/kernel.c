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



#define PRINT_SECTION_DATA(name) INFO_CORE("  " #name ": %p - %p (%v)",kernel_section_##name##_start(),kernel_section_##name##_end(),kernel_section_##name##_end()-kernel_section_##name##_start());

#define ADJUST_SECTION_FLAGS(name,flags) \
	INFO("Marking region %p - %p [%v] as %s+%s",kernel_section_##name##_start(),kernel_section_##name##_end(),pmm_align_up_address(kernel_section_##name##_end()-kernel_section_##name##_start()),(((flags)&VMM_PAGE_FLAG_READWRITE)?"RW":"RD"),(((flags)&VMM_PAGE_FLAG_NOEXECUTE)?"NX":"EX")); \
	vmm_adjust_flags(&vmm_kernel_pagemap,kernel_section_##name##_start(),(flags),VMM_PAGE_FLAG_READWRITE,pmm_align_up_address(kernel_section_##name##_end()-kernel_section_##name##_start())>>PAGE_SIZE_SHIFT)



kernel_data_t __attribute__((section(".data"))) kernel_data;



void KERNEL_NOCOVERAGE kernel_init(const kernel_data_t* bootloader_kernel_data){
	LOG_CORE("Loading kernel data...");
	kernel_data=*bootloader_kernel_data;
	INFO_CORE("Version: %lx",kernel_get_version());
	INFO_CORE("Sections:");
	PRINT_SECTION_DATA(address_range);
	PRINT_SECTION_DATA(kernel);
	PRINT_SECTION_DATA(kernel_ex);
	PRINT_SECTION_DATA(kernel_nx);
	PRINT_SECTION_DATA(kernel_rw);
	PRINT_SECTION_DATA(bss);
	INFO_CORE("Mmap Data:");
	u64 total=0;
	for (u16 i=0;i<kernel_data.mmap_size;i++){
		INFO_CORE("  %p - %p%s",(kernel_data.mmap+i)->base,(kernel_data.mmap+i)->base+(kernel_data.mmap+i)->length,((kernel_data.mmap+i)->type==1?"":" (Unusable)"));
		if ((kernel_data.mmap+i)->type==1){
			total+=(kernel_data.mmap+i)->length;
		}
	}
	INFO_CORE("Total: %v (%lu B)",total,total);
	INFO_CORE("First free address: %p",kernel_data.first_free_address);
	u8* data=(u8*)0xffffffffc01327a0;for (u8 i=0;i<96;i++){WARN("[%u] %x",i,data[i]);}
	LOG_CORE("Clearing .bss section (%v)...",kernel_section_bss_end()-kernel_section_bss_start());
	for (u64* bss=(u64*)kernel_section_bss_start();bss<(u64*)kernel_section_bss_end();bss++){
		*bss=0;
	}
	LOG("Adjusting memory flags...");
	ADJUST_SECTION_FLAGS(kernel_ex,0);
	ADJUST_SECTION_FLAGS(kernel_nx,VMM_PAGE_FLAG_NOEXECUTE);
	ADJUST_SECTION_FLAGS(kernel_rw,VMM_PAGE_FLAG_NOEXECUTE|VMM_PAGE_FLAG_READWRITE);
	ADJUST_SECTION_FLAGS(bss,VMM_PAGE_FLAG_NOEXECUTE|VMM_PAGE_FLAG_READWRITE);
}



void kernel_load(void){
	for (partition_t* partition=partition_data;partition;partition=partition->next){
		if (partition->partition_config.type!=DRIVE_TYPE_ATAPI){
			continue;
		}
		partition->flags|=PARTITION_FLAG_BOOT;
		partition_boot=partition;
		((drive_t*)(partition->drive))->flags|=DRIVE_FLAG_BOOT;
		break;
	}
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
