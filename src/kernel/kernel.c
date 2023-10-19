#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "kernel"



#define PRINT_SECTION_DATA(name) INFO("  " #name ": %p - %p (%v)",kernel_section_##name##_start(),kernel_section_##name##_end(),kernel_section_##name##_end()-kernel_section_##name##_start());

#define ADJUST_SECTION_FLAGS(name,flags) \
	INFO("Marking region %p - %p [%v] as %s+%s",kernel_section_##name##_start(),kernel_section_##name##_end(),pmm_align_up_address(kernel_section_##name##_end()-kernel_section_##name##_start()),(((flags)&VMM_PAGE_FLAG_READWRITE)?"RW":"RD"),(((flags)&VMM_PAGE_FLAG_NOEXECUTE)?"NX":"EX")); \
	vmm_adjust_flags(&vmm_kernel_pagemap,kernel_section_##name##_start(),(flags),VMM_PAGE_FLAG_READWRITE,pmm_align_up_address(kernel_section_##name##_end()-kernel_section_##name##_start())>>PAGE_SIZE_SHIFT)



kernel_data_t __attribute__((section(".data"))) kernel_data;



void KERNEL_NOCOVERAGE kernel_init(const kernel_data_t* bootloader_kernel_data){
	LOG("Loading kernel data...");
	kernel_data=*bootloader_kernel_data;
	INFO("Version: %lx",kernel_get_version());
	INFO("Sections:");
	PRINT_SECTION_DATA(kernel);
	PRINT_SECTION_DATA(kernel_ex);
	PRINT_SECTION_DATA(kernel_nx);
	PRINT_SECTION_DATA(kernel_rw);
	PRINT_SECTION_DATA(kernel_bss);
	INFO("Mmap Data:");
	u64 total=0;
	for (u16 i=0;i<kernel_data.mmap_size;i++){
		INFO("  %p - %p%s",(kernel_data.mmap+i)->base,(kernel_data.mmap+i)->base+(kernel_data.mmap+i)->length,((kernel_data.mmap+i)->type==1?"":" (Unusable)"));
		if ((kernel_data.mmap+i)->type==1){
			total+=(kernel_data.mmap+i)->length;
		}
	}
	INFO("Total: %v (%lu B)",total,total);
	INFO("First free address: %p",kernel_data.first_free_address);
	LOG("Clearing .bss section (%v)...",kernel_section_kernel_bss_end()-kernel_section_kernel_bss_start());
	for (u64* bss=(u64*)kernel_section_kernel_bss_start();bss<(u64*)kernel_section_kernel_bss_end();bss++){
		*bss=0;
	}
	LOG("Adjusting memory flags...");
	ADJUST_SECTION_FLAGS(kernel_ex,0);
	ADJUST_SECTION_FLAGS(kernel_nx,VMM_PAGE_FLAG_NOEXECUTE);
	ADJUST_SECTION_FLAGS(kernel_rw,VMM_PAGE_FLAG_NOEXECUTE|VMM_PAGE_FLAG_READWRITE);
	ADJUST_SECTION_FLAGS(kernel_bss,VMM_PAGE_FLAG_NOEXECUTE|VMM_PAGE_FLAG_READWRITE);
}



const char* kernel_lookup_symbol(u64 address,u64* offset){
	if (address<kernel_section_kernel_start()||address>=kernel_section_kernel_end()){
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
