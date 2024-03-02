#include <kernel/error/error.h>
#include <kernel/fd/fd.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/mmap/mmap.h>
#include <kernel/mp/process.h>
#include <kernel/mp/thread.h>
#include <kernel/syscall/syscall.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/vfs/node.h>
#define KERNEL_LOG_NAME "mmap"



#define MMAP_STACK_GUARD_PAGE_COUNT 2

#define USER_MEMORY_FLAG_READ 1
#define USER_MEMORY_FLAG_WRITE 2
#define USER_MEMORY_FLAG_EXEC 4
#define USER_MEMORY_FLAG_FILE 8
#define USER_MEMORY_FLAG_NOWRITEBACK 16



static pmm_counter_descriptor_t* _mmap_pmm_counter=NULL;
static omm_allocator_t* _mmap_allocator=NULL;
static omm_allocator_t* _mmap_region_allocator=NULL;



static void _dealloc_region(mmap2_t* mmap,mmap2_region_t* region){
	rb_tree_remove_node(&(mmap->address_tree),&(region->rb_node));
	u64 guard_page_size=((region->flags&MMAP2_REGION_FLAG_STACK)?MMAP_STACK_GUARD_PAGE_COUNT<<PAGE_SIZE_SHIFT:0);
	WARN("Push free region: %p, %v",region->rb_node.key-guard_page_size,region->length+guard_page_size);
	omm_dealloc(_mmap_region_allocator,region);
}



KERNEL_EARLY_EARLY_INIT(){
	LOG("Initializing mmap allocator...");
	_mmap_pmm_counter=pmm_alloc_counter("mmap");
	_mmap_allocator=omm_init("mmap",sizeof(mmap2_t),8,4,pmm_alloc_counter("omm_mmap"));
	spinlock_init(&(_mmap_allocator->lock));
	_mmap_region_allocator=omm_init("mmap_region",sizeof(mmap2_region_t),8,4,pmm_alloc_counter("omm_mmap_region"));
	spinlock_init(&(_mmap_region_allocator->lock));
}



mmap2_t* mmap2_init(vmm_pagemap_t* pagemap,u64 bottom_address,u64 top_address){
	mmap2_t* out=omm_alloc(_mmap_allocator);
	spinlock_init(&(out->lock));
	out->pagemap=pagemap;
	out->bottom_address=bottom_address;
	out->break_address=bottom_address;
	out->heap_address=top_address;
	out->top_address=top_address;
	rb_tree_init(&(out->address_tree));
	return out;
}



void mmap2_deinit(mmap2_t* mmap){
	omm_dealloc(_mmap_allocator,mmap);
}



KERNEL_PUBLIC mmap2_region_t* mmap2_alloc(mmap2_t* mmap,u64 address,u64 length,u32 flags,vfs_node_t* file){
	if ((address|length)&(PAGE_SIZE-1)){
		return NULL;
	}
	if (!length&&file){
		length=pmm_align_up_address(vfs_node_resize(file,0,VFS_NODE_FLAG_RESIZE_RELATIVE));
	}
	if (!length){
		return NULL;
	}
	u64 guard_page_size=((flags&MMAP2_REGION_FLAG_STACK)?MMAP_STACK_GUARD_PAGE_COUNT<<PAGE_SIZE_SHIFT:0);
	length+=guard_page_size;
	spinlock_acquire_exclusive(&(mmap->lock));
	if (!address){
		mmap->heap_address-=length;
		address=mmap->heap_address;
	}
	else if (!(flags&MMAP2_REGION_FLAG_FORCE)){
		panic("mmap2_alloc: check user address and length");
	}
	mmap2_region_t* out=omm_alloc(_mmap_region_allocator);
	out->rb_node.key=address+guard_page_size;
	out->length=length-guard_page_size;
	out->flags=flags;
	out->file=file;
	rb_tree_insert_node(&(mmap->address_tree),&(out->rb_node));
	spinlock_release_exclusive(&(mmap->lock));
	if (flags&MMAP2_REGION_FLAG_COMMIT){
		for (u64 offset=address+guard_page_size;offset<address+length;offset+=PAGE_SIZE){
			mmap2_handle_pf(mmap,offset);
		}
	}
	return out;
}



KERNEL_PUBLIC _Bool mmap2_dealloc(mmap2_t* mmap,u64 address,u64 length){
	spinlock_acquire_exclusive(&(mmap->lock));
	mmap2_region_t* region=(void*)rb_tree_lookup_decreasing_node(&(mmap->address_tree),address);
	if (!region||region->rb_node.key+region->length<=address){
		spinlock_release_exclusive(&(mmap->lock));
		return 0;
	}
	if (!length){
		length=region->rb_node.key+region->length-address;
	}
	if (region->rb_node.key!=address||region->length!=length){
		panic("mmap2_dealloc: partial dealloc");
	}
	_dealloc_region(mmap,region);
	spinlock_release_exclusive(&(mmap->lock));
	return 1;
}



KERNEL_PUBLIC void mmap2_dealloc_region(mmap2_t* mmap,mmap2_region_t* region){
	spinlock_acquire_exclusive(&(mmap->lock));
	_dealloc_region(mmap,region);
	spinlock_release_exclusive(&(mmap->lock));
}



KERNEL_PUBLIC mmap2_region_t* mmap2_lookup(mmap2_t* mmap,u64 address){
	if (!mmap){
		return NULL;
	}
	spinlock_acquire_exclusive(&(mmap->lock));
	mmap2_region_t* out=(void*)rb_tree_lookup_decreasing_node(&(mmap->address_tree),address);
	if (out&&out->rb_node.key+out->length<=address){
		out=NULL;
	}
	spinlock_release_exclusive(&(mmap->lock));
	return out;
}



KERNEL_PUBLIC mmap2_region_t* mmap2_map_to_kernel(mmap2_t* mmap,u64 address,u64 length){
	mmap2_region_t* out=mmap2_alloc(process_kernel->mmap2,0,length,MMAP2_REGION_FLAG_EXTERNAL|MMAP2_REGION_FLAG_VMM_WRITE,NULL);
	for (u64 offset=address;offset<address+length;offset+=PAGE_SIZE){
		u64 physical_address=vmm_virtual_to_physical(mmap->pagemap,offset);
		if (!physical_address){
			physical_address=mmap2_handle_pf(mmap,offset);
			if (!physical_address){
				panic("mmap2_map_to_kernel: invalid address");
			}
		}
		vmm_map_page(&(process_kernel->pagemap),physical_address,out->rb_node.key+offset-address,VMM_PAGE_FLAG_NOEXECUTE|VMM_PAGE_FLAG_READWRITE|VMM_PAGE_FLAG_PRESENT);
	}
	return out;
}



u64 mmap2_handle_pf(mmap2_t* mmap,u64 address){
	if (!mmap){
		return 0;
	}
	spinlock_acquire_exclusive(&(mmap->lock));
	mmap2_region_t* region=(void*)rb_tree_lookup_decreasing_node(&(mmap->address_tree),address);
	if (region&&region->rb_node.key+region->length>address){
		u64 out=pmm_alloc(1,_mmap_pmm_counter,0);
		if (region->file){
			vfs_node_read(region->file,address-region->rb_node.key,(void*)(out+VMM_HIGHER_HALF_ADDRESS_OFFSET),PAGE_SIZE,0);
		}
		u64 flags=VMM_PAGE_FLAG_PRESENT;
		if (region->flags&MMAP2_REGION_FLAG_VMM_USER){
			flags|=VMM_PAGE_FLAG_USER;
		}
		if (region->flags&MMAP2_REGION_FLAG_VMM_WRITE){
			flags|=VMM_PAGE_FLAG_READWRITE;
		}
		if (!(region->flags&MMAP2_REGION_FLAG_VMM_EXEC)){
			flags|=VMM_PAGE_FLAG_NOEXECUTE;
		}
		vmm_map_page(mmap->pagemap,out,address,flags);
		spinlock_release_exclusive(&(mmap->lock));
		return out;
	}
	spinlock_release_exclusive(&(mmap->lock));
	return 0;
}



error_t syscall_memory_map(u64 size,u64 flags,handle_id_t fd){
	u64 mmap_flags=MMAP2_REGION_FLAG_VMM_USER;
	vfs_node_t* file=NULL;
	if (flags&USER_MEMORY_FLAG_WRITE){
		mmap_flags|=MMAP2_REGION_FLAG_VMM_WRITE;
	}
	if (flags&USER_MEMORY_FLAG_EXEC){
		mmap_flags|=MMAP2_REGION_FLAG_VMM_EXEC;
	}
	if (flags&USER_MEMORY_FLAG_FILE){
		file=fd_get_node(fd);
		if (!file){
			return ERROR_INVALID_HANDLE;
		}
	}
	if (flags&USER_MEMORY_FLAG_NOWRITEBACK){
		mmap_flags|=MMAP2_REGION_FLAG_NO_WRITEBACK;
	}
	mmap2_region_t* out=mmap2_alloc(THREAD_DATA->process->mmap2,0,pmm_align_up_address(size),mmap_flags,file);
	return (out?out->rb_node.key:ERROR_NO_MEMORY);
}



error_t syscall_memory_change_flags(u64 address,u64 size,u64 flags){
	u64 vmm_set_flags=0;
	if (flags&USER_MEMORY_FLAG_WRITE){
		vmm_set_flags|=VMM_PAGE_FLAG_READWRITE;
	}
	if (!(flags&USER_MEMORY_FLAG_EXEC)){
		vmm_set_flags|=VMM_PAGE_FLAG_NOEXECUTE;
	}
	size=pmm_align_up_address(size+(address&(PAGE_SIZE-1)));
	address=pmm_align_down_address(address);
	mmap2_t* mmap=THREAD_DATA->process->mmap2;
	for (u64 offset=address;offset<address+size;offset+=PAGE_SIZE){
		if (!vmm_virtual_to_physical(mmap->pagemap,offset)&&!mmap2_handle_pf(mmap,offset)){
			return ERROR_INVALID_ARGUMENT(0);
		}
		vmm_adjust_flags(mmap->pagemap,offset,vmm_set_flags,VMM_PAGE_FLAG_NOEXECUTE|VMM_PAGE_FLAG_READWRITE,1,1);
	}
	return ERROR_OK;
}



error_t syscall_memory_unmap(u64 address,u64 size){
	return (mmap2_dealloc(THREAD_DATA->process->mmap2,pmm_align_down_address(address),pmm_align_up_address(size+(address&(PAGE_SIZE-1))))?ERROR_OK:ERROR_INVALID_ARGUMENT(0));
}
