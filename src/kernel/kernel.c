#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/memory/amm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/mp/event.h>
#include <kernel/mp/thread.h>
#include <kernel/time/time.h>
#include <kernel/types.h>
#include <kernel/util/memory.h>
#define KERNEL_LOG_NAME "kernel"



#define PRINT_SECTION_DATA(name) INFO("  "#name": %p - %p (%v)",kernel_section_##name##_start(),kernel_section_##name##_end(),kernel_section_##name##_end()-kernel_section_##name##_start())

#define ADJUST_SECTION_FLAGS(name,flags) \
	INFO("Marking region %p - %p [%v] as %s+%s",kernel_section_##name##_start(),kernel_section_##name##_end(),pmm_align_up_address(kernel_section_##name##_end()-kernel_section_##name##_start()),(((flags)&VMM_PAGE_FLAG_READWRITE)?"RW":"RD"),(((flags)&VMM_PAGE_FLAG_NOEXECUTE)?"NX":"EX")); \
	vmm_adjust_flags(&vmm_kernel_pagemap,kernel_section_##name##_start(),(flags),VMM_PAGE_FLAG_READWRITE,pmm_align_up_address(kernel_section_##name##_end()-kernel_section_##name##_start())>>PAGE_SIZE_SHIFT,1)



static void _unmap_section(u64 start,u64 end){
	INFO("Unmapping region %p - %p [%v]",start,end,pmm_align_up_address(end-start));
	vmm_adjust_flags(&vmm_kernel_pagemap,start,VMM_PAGE_FLAG_READWRITE,0,pmm_align_up_address(end-start)>>PAGE_SIZE_SHIFT,1);
	mem_fill((void*)start,end-start,0);
	for (;start<end;start+=PAGE_SIZE){
		vmm_unmap_page(&vmm_kernel_pagemap,start);
	}
}



static volatile const u64 KERNEL_INIT_WRITE __kernel_version=0;
static volatile const char KERNEL_INIT_WRITE __kernel_build_name[64]="";

static u8 KERNEL_INIT_WRITE _kernel_boot_uuid[16];

kernel_data_t KERNEL_EARLY_WRITE kernel_data;



void KERNEL_EARLY_EXEC kernel_init(const kernel_data_t* bootloader_kernel_data){
	LOG("Loading kernel data...");
	kernel_data=*bootloader_kernel_data;
	INFO("Version: %lx",kernel_get_version());
	INFO("Build: %s",kernel_get_build_name());
	INFO("Sections:");
	PRINT_SECTION_DATA(kernel);
	PRINT_SECTION_DATA(kernel_ue);
	PRINT_SECTION_DATA(kernel_ur);
	PRINT_SECTION_DATA(kernel_uw);
	PRINT_SECTION_DATA(kernel_ex);
	PRINT_SECTION_DATA(kernel_nx);
	PRINT_SECTION_DATA(kernel_rw);
	PRINT_SECTION_DATA(kernel_iw);
	INFO("Mmap Data:");
	u64 total=0;
	for (u16 i=0;i<kernel_data.mmap_size;i++){
		INFO("  %p - %p",(kernel_data.mmap+i)->base,(kernel_data.mmap+i)->base+(kernel_data.mmap+i)->length);
		total+=(kernel_data.mmap+i)->length;
	}
	INFO("Total: %v (%lu B)",total,total);
	INFO("First free address: %p",kernel_data.first_free_address);
	mem_copy(_kernel_boot_uuid,kernel_data.boot_fs_uuid,16);
	INFO("Boot filesystem UUID: %g",_kernel_boot_uuid);
}



void KERNEL_EARLY_EXEC kernel_adjust_memory_flags(void){
	LOG("Adjusting memory flags...");
	ADJUST_SECTION_FLAGS(kernel_ue,0);
	ADJUST_SECTION_FLAGS(kernel_ur,VMM_PAGE_FLAG_NOEXECUTE);
	ADJUST_SECTION_FLAGS(kernel_uw,VMM_PAGE_FLAG_NOEXECUTE|VMM_PAGE_FLAG_READWRITE);
	ADJUST_SECTION_FLAGS(kernel_ex,0);
	ADJUST_SECTION_FLAGS(kernel_nx,VMM_PAGE_FLAG_NOEXECUTE);
	ADJUST_SECTION_FLAGS(kernel_rw,VMM_PAGE_FLAG_NOEXECUTE|VMM_PAGE_FLAG_READWRITE);
	ADJUST_SECTION_FLAGS(kernel_iw,VMM_PAGE_FLAG_NOEXECUTE|VMM_PAGE_FLAG_READWRITE);
}



void KERNEL_EARLY_EXEC kernel_early_execute_initializers(void){
	for (const kernel_initializer_t* func=(void*)kernel_section_early_early_initializers_start();(u64)func<kernel_section_early_early_initializers_end();func++){
		(*func)();
	}
	for (const kernel_initializer_t* func=(void*)kernel_section_early_initializers_start();(u64)func<kernel_section_early_initializers_end();func++){
		(*func)();
	}
}



void KERNEL_AWAITS_EARLY kernel_execute_initializers(void){
	u32 thread_count=(kernel_section_async_initializers_end()-kernel_section_async_initializers_start())/sizeof(const kernel_initializer_t);
	thread_t** threads=amm_alloc(thread_count*sizeof(thread_t*));
	u32 i=0;
	for (const kernel_initializer_t* func=(void*)kernel_section_async_initializers_start();(u64)func<kernel_section_async_initializers_end();func++){
		threads[i]=thread_create_kernel_thread(NULL,"kernel.init",*func,0);
		i++;
	}
	for (const kernel_initializer_t* func=(void*)kernel_section_initializers_start();(u64)func<kernel_section_initializers_end();func++){
		(*func)();
	}
	for (i=0;i<thread_count;i++){
		event_await(&(threads[i]->termination_event),1,0);
		handle_release(&(threads[i]->handle));
	}
	amm_dealloc(threads);
}



void kernel_adjust_memory_flags_after_init(void){
	LOG("Adjusting memory flags (after init)...");
	_unmap_section(kernel_section_kernel_ue_start(),kernel_section_kernel_ue_end());
	_unmap_section(kernel_section_kernel_ur_start(),kernel_section_kernel_ur_end());
	_unmap_section(kernel_section_kernel_uw_start(),kernel_section_kernel_uw_end());
	ADJUST_SECTION_FLAGS(kernel_iw,VMM_PAGE_FLAG_NOEXECUTE);
}



KERNEL_PUBLIC u64 kernel_gcov_info_data(u64* size){
	*size=kernel_section_gcov_info_end()-kernel_section_gcov_info_start();
	return kernel_section_gcov_info_start();
}



KERNEL_PUBLIC u64 kernel_get_version(void){
	return __kernel_version;
}



KERNEL_PUBLIC const char* kernel_get_build_name(void){
	return (const char*)__kernel_build_name;
}



KERNEL_PUBLIC const u8* kernel_get_boot_uuid(void){
	return _kernel_boot_uuid;
}
