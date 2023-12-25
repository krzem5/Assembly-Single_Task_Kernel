#include <kernel/fd/fd.h>
#include <kernel/handle/handle.h>
#include <kernel/memory/mmap.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/mp/thread.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>



#define MEMORY_FLAG_READ 1
#define MEMORY_FLAG_WRITE 2
#define MEMORY_FLAG_EXEC 4
#define MEMORY_FLAG_FILE 8
#define MEMORY_FLAG_NOWRITEBACK 16



static pmm_counter_descriptor_t* _user_data_pmm_counter=NULL;



u64 syscall_memory_map(u64 size,u64 flags,handle_id_t fd){
	if (!_user_data_pmm_counter){
		_user_data_pmm_counter=pmm_alloc_counter("user_data");
	}
	u64 mmap_flags=MMAP_REGION_FLAG_VMM_USER;
	vfs_node_t* file=NULL;
	if (flags&MEMORY_FLAG_WRITE){
		mmap_flags|=MMAP_REGION_FLAG_VMM_READWRITE;
	}
	if (!(flags&MEMORY_FLAG_EXEC)){
		mmap_flags|=MMAP_REGION_FLAG_VMM_NOEXECUTE;
	}
	if (flags&MEMORY_FLAG_FILE){
		file=fd_get_node(fd);
		if (!file){
			return 0;
		}
	}
	if (flags&MEMORY_FLAG_NOWRITEBACK){
		mmap_flags|=MMAP_REGION_FLAG_NO_FILE_WRITEBACK;
	}
	mmap_region_t* out=mmap_alloc(&(THREAD_DATA->process->mmap),0,pmm_align_up_address(size),_user_data_pmm_counter,mmap_flags,file);
	return (out?out->rb_node.key:0);
}



u64 syscall_memory_change_flags(u64 address,u64 size,u64 flags){
	u64 mmap_flags=0;
	if (flags&MEMORY_FLAG_WRITE){
		mmap_flags|=VMM_PAGE_FLAG_READWRITE;
	}
	if (!(flags&MEMORY_FLAG_EXEC)){
		mmap_flags|=VMM_PAGE_FLAG_NOEXECUTE;
	}
	return mmap_change_flags(&(THREAD_DATA->process->mmap),pmm_align_down_address(address),pmm_align_up_address(size+(address&(PAGE_SIZE-1))),mmap_flags,VMM_PAGE_FLAG_NOEXECUTE|VMM_PAGE_FLAG_READWRITE);
}



u64 syscall_memory_unmap(u64 address,u64 size){
	return mmap_dealloc(&(THREAD_DATA->process->mmap),pmm_align_down_address(address),pmm_align_up_address(size+(address&(PAGE_SIZE-1))));
}
