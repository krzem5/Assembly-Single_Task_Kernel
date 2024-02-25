#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/random/random.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "aslr"



#define KERNEL_ASLR_KERNEL_END (kernel_get_offset()+0x8000000)
#define KERNEL_ASLR_MODULE_START (kernel_get_offset()+0xc000000)

volatile const u64* KERNEL_EARLY_READ __kernel_relocation_data=NULL;

u64 KERNEL_EARLY_WRITE aslr_module_base=0;



void KERNEL_EARLY_EXEC aslr_reloc_kernel(void){
	LOG("Relocating kernel...");
	u64 kernel_size=pmm_align_up_address(kernel_data.first_free_address)+kernel_get_offset()-kernel_section_kernel_start();
	u64 offset;
	random_generate(&offset,sizeof(u64));
	offset=pmm_align_down_address(offset%(KERNEL_ASLR_KERNEL_END-kernel_size-kernel_get_offset()-kernel_data.first_free_address-1));
	INFO("Kernel base: %p",kernel_data.first_free_address+offset+kernel_get_offset());
	for (u64 i=kernel_section_kernel_start();i<kernel_data.first_free_address+kernel_get_offset();i+=PAGE_SIZE){
		vmm_map_page(&vmm_kernel_pagemap,i-kernel_get_offset(),i+offset,VMM_PAGE_FLAG_READWRITE|VMM_PAGE_FLAG_PRESENT);
	}
	for (u64 i=0;__kernel_relocation_data[i];i++){
		*((u32*)(__kernel_relocation_data[i]))+=offset;
	}
	random_generate(&aslr_module_base,sizeof(u64));
	aslr_module_base=KERNEL_ASLR_KERNEL_END+pmm_align_down_address(aslr_module_base%(KERNEL_ASLR_MODULE_START-KERNEL_ASLR_KERNEL_END));
	INFO("Module base: %p",aslr_module_base);
}
