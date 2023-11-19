#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/time/time.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "kernel"



#define PRINT_SECTION_DATA(name) INFO("  " #name ": %p - %p (%v)",kernel_section_##name##_start(),kernel_section_##name##_end(),kernel_section_##name##_end()-kernel_section_##name##_start());

#define ADJUST_SECTION_FLAGS(name,flags) \
	INFO("Marking region %p - %p [%v] as %s+%s",kernel_section_##name##_start(),kernel_section_##name##_end(),pmm_align_up_address(kernel_section_##name##_end()-kernel_section_##name##_start()),(((flags)&VMM_PAGE_FLAG_READWRITE)?"RW":"RD"),(((flags)&VMM_PAGE_FLAG_NOEXECUTE)?"NX":"EX")); \
	vmm_adjust_flags(&vmm_kernel_pagemap,kernel_section_##name##_start(),(flags),VMM_PAGE_FLAG_READWRITE,pmm_align_up_address(kernel_section_##name##_end()-kernel_section_##name##_start())>>PAGE_SIZE_SHIFT,1)



kernel_data_t KERNEL_INIT_WRITE kernel_data;



void KERNEL_NOCOVERAGE kernel_init(const kernel_data_t* bootloader_kernel_data){
	LOG("Loading kernel data...");
	LOG("Clearing .bss section (%v)...",kernel_section_kernel_zw_end()-kernel_section_kernel_zw_start());
	for (u64* bss=(u64*)kernel_section_kernel_zw_start();bss<(u64*)kernel_section_kernel_zw_end();bss++){
		*bss=0;
	}
	kernel_data=*bootloader_kernel_data;
	INFO("Version: %lx",kernel_get_version());
	INFO("Sections:");
	PRINT_SECTION_DATA(kernel);
	PRINT_SECTION_DATA(kernel_ex);
	PRINT_SECTION_DATA(kernel_nx);
	PRINT_SECTION_DATA(kernel_rw);
	PRINT_SECTION_DATA(kernel_iw);
	PRINT_SECTION_DATA(kernel_zw);
	INFO("Mmap Data:");
	u64 total=0;
	for (u16 i=0;i<kernel_data.mmap_size;i++){
		INFO("  %p - %p",(kernel_data.mmap+i)->base,(kernel_data.mmap+i)->base+(kernel_data.mmap+i)->length);
		total+=(kernel_data.mmap+i)->length;
	}
	INFO("Total: %v (%lu B)",total,total);
	INFO("First free address: %p",kernel_data.first_free_address);
}



void kernel_adjust_memory_flags(void){
	LOG("Adjusting memory flags...");
	ADJUST_SECTION_FLAGS(kernel_ex,0);
	ADJUST_SECTION_FLAGS(kernel_nx,VMM_PAGE_FLAG_NOEXECUTE);
	ADJUST_SECTION_FLAGS(kernel_rw,VMM_PAGE_FLAG_NOEXECUTE|VMM_PAGE_FLAG_READWRITE);
	ADJUST_SECTION_FLAGS(kernel_iw,VMM_PAGE_FLAG_NOEXECUTE|VMM_PAGE_FLAG_READWRITE);
	ADJUST_SECTION_FLAGS(kernel_zw,VMM_PAGE_FLAG_NOEXECUTE|VMM_PAGE_FLAG_READWRITE);
}



void kernel_adjust_memory_flags_after_init(void){
	LOG("Adjusting memory flags (after init)...");
	ADJUST_SECTION_FLAGS(kernel_iw,VMM_PAGE_FLAG_NOEXECUTE);
}



u64 kernel_gcov_info_data(u64* size){
	*size=kernel_section_gcov_info_end()-kernel_section_gcov_info_start();
	return kernel_section_gcov_info_start();
}
