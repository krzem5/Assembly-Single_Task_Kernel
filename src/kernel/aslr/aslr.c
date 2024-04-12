#include <kernel/aslr/aslr.h>
#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/random/random.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "aslr"



#define KERNEL_ASLR_KERNEL_END 0xffffffffc8000000ull
#define KERNEL_ASLR_MODULE_START 0xffffffffcc000000ull



static u64 KERNEL_EARLY_WRITE _aslr_offset=0;

volatile const u64* KERNEL_EARLY_READ __kernel_relocation_data=NULL;

u64 KERNEL_EARLY_WRITE aslr_module_base=0;
u64 KERNEL_EARLY_WRITE aslr_module_size=0;



static void KERNEL_EARLY_EXEC KERNEL_NORETURN _finish_relocation(void (*KERNEL_NORETURN next_stage_callback)(void)){
	INFO("Unmapping default kernel location...");
	for (u64 i=kernel_section_kernel_start();i<kernel_section_kernel_end();i+=PAGE_SIZE){
		vmm_unmap_page(&vmm_kernel_pagemap,i-_aslr_offset);
	}
	_aslr_offset=0;
	next_stage_callback();
}



u64 aslr_generate_address(u64 min,u64 max){
#ifndef KERNEL_RELEASE
	return min;
#else
	u64 value=0;
	random_generate(&value,sizeof(u64));
	return min+pmm_align_down_address(value%(max-min));
#endif
}



void KERNEL_EARLY_EXEC KERNEL_NORETURN aslr_reloc_kernel(void (*KERNEL_NORETURN next_stage_callback)(void)){
#ifndef KERNEL_RELEASE
	ERROR("ASLR disabled");
	aslr_module_base=KERNEL_ASLR_MODULE_START;
	aslr_module_size=-PAGE_SIZE*2-KERNEL_ASLR_MODULE_START;
	INFO("Kernel range: %p - %p",kernel_section_kernel_start(),kernel_section_kernel_end());
	INFO("Module range: %p - %p",aslr_module_base,aslr_module_base+aslr_module_size);
	(void)_finish_relocation;
	next_stage_callback();
#else
	LOG("Generating ASLR addresses...");
	u64 kernel_size=pmm_align_up_address(kernel_section_kernel_end()-kernel_section_kernel_start());
	random_generate(&_aslr_offset,sizeof(u64));
	_aslr_offset=pmm_align_down_address(_aslr_offset%(KERNEL_ASLR_KERNEL_END-kernel_size-kernel_section_kernel_end()-1));
	aslr_module_base=aslr_generate_address(KERNEL_ASLR_KERNEL_END,KERNEL_ASLR_MODULE_START);
	aslr_module_size=-PAGE_SIZE*2-KERNEL_ASLR_MODULE_START;
	INFO("Kernel range: %p - %p",kernel_section_kernel_end()+_aslr_offset,kernel_section_kernel_end()+_aslr_offset+kernel_size);
	INFO("Module range: %p - %p",aslr_module_base,aslr_module_base+aslr_module_size);
	LOG("Relocating kernel...");
	for (u64 i=kernel_section_kernel_start();i<kernel_section_kernel_end();i+=PAGE_SIZE){
		vmm_map_page(&vmm_kernel_pagemap,i-kernel_get_offset(),i+_aslr_offset,VMM_PAGE_FLAG_READWRITE|VMM_PAGE_FLAG_PRESENT);
	}
	for (u64 i=0;__kernel_relocation_data[i];i++){
		*((u32*)(__kernel_relocation_data[i]))+=_aslr_offset;
	}
	_aslr_adjust_rip(next_stage_callback+_aslr_offset,_finish_relocation);
#endif
}
