#include <kernel/format/format.h>
#include <kernel/handle/handle.h>
#include <kernel/kernel.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/mmap.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/mp/process.h>
#include <kernel/mp/thread_list.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "process"



#define USERSPACE_LOWEST_ADDRESS 0x0000000000001000ull
#define USERSPACE_HIGHEST_ADDRESS 0x0000800000000000ull

#define KERNELSPACE_LOWEST_ADDRESS 0xfffff00000000000ull



static pmm_counter_descriptor_t _process_omm_pmm_counter=PMM_COUNTER_INIT_STRUCT("omm_process");
static omm_allocator_t* _process_allocator=NULL;



HANDLE_DECLARE_TYPE(PROCESS,{
	process_t* process=handle->object;
	if (process->thread_list.head){
		panic("Unterminated process not referenced");
	}
	mmap_deinit(&(process->mmap));
	vmm_pagemap_deinit(&(process->pagemap));
	omm_dealloc(_process_allocator,process);
});

process_t* process_kernel;
mmap_t process_kernel_image_mmap;



void process_init(void){
	LOG("Creating kernel process...");
	_process_allocator=omm_init("process",sizeof(process_t),8,2,&_process_omm_pmm_counter);
	spinlock_init(&(_process_allocator->lock));
	process_kernel=omm_alloc(_process_allocator);
	handle_new(process_kernel,HANDLE_TYPE_PROCESS,&(process_kernel->handle));
	handle_acquire(&(process_kernel->handle));
	spinlock_init(&(process_kernel->lock));
	vmm_pagemap_init(&(process_kernel->pagemap));
	mmap_init(&vmm_kernel_pagemap,KERNELSPACE_LOWEST_ADDRESS,kernel_get_offset(),&(process_kernel->mmap));
	thread_list_init(&(process_kernel->thread_list));
	process_kernel->name=smm_alloc("kernel",0);
	process_kernel->image=smm_alloc("/boot/kernel.bin",0);
	process_kernel->uid=0;
	process_kernel->gid=0;
	mmap_init(&vmm_kernel_pagemap,kernel_get_offset(),-PAGE_SIZE,&process_kernel_image_mmap);
	if (!mmap_alloc(&process_kernel_image_mmap,kernel_get_offset(),kernel_data.first_free_address,NULL,0,NULL)){
		panic("Unable to reserve kernel memory");
	}
	handle_finish_setup(&(process_kernel->handle));
}



process_t* process_new(const char* image,const char* name){
	process_t* out=omm_alloc(_process_allocator);
	handle_new(out,HANDLE_TYPE_PROCESS,&(out->handle));
	spinlock_init(&(out->lock));
	vmm_pagemap_init(&(out->pagemap));
	mmap_init(&(out->pagemap),USERSPACE_LOWEST_ADDRESS,USERSPACE_HIGHEST_ADDRESS,&(out->mmap));
	thread_list_init(&(out->thread_list));
	out->name=smm_alloc(name,0);
	out->image=smm_alloc(image,0);
	out->uid=0;
	out->gid=0;
	handle_finish_setup(&(out->handle));
	return out;
}
