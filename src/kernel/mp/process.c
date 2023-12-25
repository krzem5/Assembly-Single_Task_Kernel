#include <kernel/acl/acl.h>
#include <kernel/cpu/cpu.h>
#include <kernel/elf/elf.h>
#include <kernel/error/error.h>
#include <kernel/format/format.h>
#include <kernel/handle/handle.h>
#include <kernel/id/user.h>
#include <kernel/kernel.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/mmap.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/mp/event.h>
#include <kernel/mp/process.h>
#include <kernel/mp/thread.h>
#include <kernel/mp/thread_list.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "process"



#define USERSPACE_LOWEST_ADDRESS 0x0000000000001000ull
#define USERSPACE_HIGHEST_ADDRESS 0x0000800000000000ull

#define KERNELSPACE_LOWEST_ADDRESS 0xfffff00000000000ull



static omm_allocator_t* KERNEL_INIT_WRITE _process_allocator=NULL;

KERNEL_PUBLIC handle_type_t process_handle_type;
KERNEL_PUBLIC process_t* KERNEL_INIT_WRITE process_kernel;
KERNEL_PUBLIC mmap_t process_kernel_image_mmap;



static void _process_handle_destructor(handle_t* handle){
	process_t* process=handle->object;
	if (process->thread_list.head){
		panic("Unterminated process not referenced");
	}
	event_dispatch(process->event,EVENT_DISPATCH_FLAG_DISPATCH_ALL|EVENT_DISPATCH_FLAG_SET_ACTIVE|EVENT_DISPATCH_FLAG_BYPASS_ACL);
	mmap_deinit(&(process->mmap));
	vmm_pagemap_deinit(&(process->pagemap));
	omm_dealloc(_process_allocator,process);
}



KERNEL_EARLY_INIT(){
	LOG("Creating kernel process...");
	_process_allocator=omm_init("process",sizeof(process_t),8,2,pmm_alloc_counter("omm_process"));
	spinlock_init(&(_process_allocator->lock));
	process_handle_type=handle_alloc("process",_process_handle_destructor);
	process_kernel=omm_alloc(_process_allocator);
	handle_new(process_kernel,process_handle_type,&(process_kernel->handle));
	process_kernel->handle.acl=acl_create();
	handle_acquire(&(process_kernel->handle));
	spinlock_init(&(process_kernel->lock));
	vmm_pagemap_init(&(process_kernel->pagemap));
	mmap_init(&vmm_kernel_pagemap,KERNELSPACE_LOWEST_ADDRESS,kernel_get_offset(),&(process_kernel->mmap));
	thread_list_init(&(process_kernel->thread_list));
	process_kernel->name=smm_alloc("kernel",0);
	process_kernel->image=smm_alloc("/boot/kernel.bin",0);
	process_kernel->uid=0;
	process_kernel->gid=0;
	process_kernel->event=event_create();
	mmap_init(&vmm_kernel_pagemap,kernel_get_offset(),-PAGE_SIZE,&process_kernel_image_mmap);
	if (!mmap_alloc(&process_kernel_image_mmap,kernel_get_offset(),kernel_data.first_free_address,NULL,0,NULL)){
		panic("Unable to reserve kernel memory");
	}
	handle_finish_setup(&(process_kernel->handle));
}



KERNEL_PUBLIC process_t* process_create(const char* image,const char* name){
	process_t* out=omm_alloc(_process_allocator);
	handle_new(out,process_handle_type,&(out->handle));
	process_kernel->handle.acl=acl_create();
	if (CPU_HEADER_DATA->current_thread){
		acl_set(process_kernel->handle.acl,THREAD_DATA->process,0,PROCESS_ACL_FLAG_CREATE_THREAD|PROCESS_ACL_FLAG_TERMINATE);
	}
	spinlock_init(&(out->lock));
	vmm_pagemap_init(&(out->pagemap));
	mmap_init(&(out->pagemap),USERSPACE_LOWEST_ADDRESS,USERSPACE_HIGHEST_ADDRESS,&(out->mmap));
	thread_list_init(&(out->thread_list));
	out->name=smm_alloc(name,0);
	out->image=smm_alloc(image,0);
	out->uid=0;
	out->gid=0;
	out->event=event_create();
	handle_finish_setup(&(out->handle));
	return out;
}



KERNEL_PUBLIC _Bool process_is_root(void){
	return !THREAD_DATA->process->uid||!THREAD_DATA->process->gid||uid_has_group(THREAD_DATA->process->uid,0);
}



error_t syscall_process_get_pid(void){
	return THREAD_DATA->process->handle.rb_node.key;
}



error_t syscall_process_start(const char* path,u32 argc,const char*const* argv,const char*const* environ,u32 flags){
	if (!syscall_get_string_length(path)){
		return ERROR_INVALID_ARGUMENT(0);
	}
	if (argc*sizeof(u64)>syscall_get_user_pointer_max_length(argv)){
		return ERROR_INVALID_ARGUMENT(1);
	}
	for (u64 i=0;i<argc;i++){
		if (!syscall_get_string_length(argv[i])){
			return ERROR_INVALID_ARGUMENT(1);
		}
	}
	// copy all vars to a temp buffer + check environ for overflow
	handle_id_t out=elf_load(path,argc,argv,environ,flags);
	return (out?out:ERROR_NOT_FOUND);
}



error_t syscall_process_get_event(handle_id_t process_handle){
	handle_t* handle=handle_lookup_and_acquire(process_handle,process_handle_type);
	if (!handle){
		return ERROR_INVALID_HANDLE;
	}
	process_t* process=handle->object;
	u64 out=process->event->handle.rb_node.key;
	handle_release(handle);
	return out;
}
