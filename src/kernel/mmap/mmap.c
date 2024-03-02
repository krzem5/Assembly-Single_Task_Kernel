#include <kernel/error/error.h>
#include <kernel/fd/fd.h>
#include <kernel/isr/pf.h>
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



#define MMAP_STACK_GUARD_PAGE_COUNT 4

#define USER_MEMORY_FLAG_READ 1
#define USER_MEMORY_FLAG_WRITE 2
#define USER_MEMORY_FLAG_EXEC 4
#define USER_MEMORY_FLAG_FILE 8
#define USER_MEMORY_FLAG_NOWRITEBACK 16
#define USER_MEMORY_FLAG_STACK 32



static pmm_counter_descriptor_t* _mmap_pmm_counter=NULL;
static omm_allocator_t* _mmap_allocator=NULL;
static omm_allocator_t* _mmap_region_allocator=NULL;
static omm_allocator_t* _mmap_length_group_allocator=NULL;
static omm_allocator_t* _mmap_free_region_allocator=NULL;



static void _pop_free_region(mmap_t* mmap,mmap_free_region_t* free_region){
	mmap_length_group_t* group=free_region->group;
	rb_tree_remove_node(&(mmap->free_address_tree),&(free_region->rb_node));
	if (free_region->prev){
		free_region->prev->next=free_region->next;
	}
	else{
		group->head=free_region->next;
		if (!group->head){
			rb_tree_remove_node(&(mmap->length_tree),&(group->rb_node));
			omm_dealloc(_mmap_length_group_allocator,group);
		}
	}
	if (free_region->next){
		free_region->next->prev=free_region->prev;
	}
	omm_dealloc(_mmap_free_region_allocator,free_region);
}



static void _push_free_region(mmap_t* mmap,u64 address,u64 length){
	mmap_free_region_t* free_region=(void*)rb_tree_lookup_node(&(mmap->free_address_tree),address+length);
	if (free_region){
		WARN("Coalesce ahead");
	}
	free_region=(void*)rb_tree_lookup_decreasing_node(&(mmap->free_address_tree),address);
	if (free_region&&free_region->rb_node.key+free_region->group->rb_node.key==address){
		WARN("Coalesce behind");
	}
	mmap_length_group_t* group=(void*)rb_tree_lookup_node(&(mmap->length_tree),length);
	if (!group){
		group=omm_alloc(_mmap_length_group_allocator);
		group->rb_node.key=length;
		group->head=NULL;
		rb_tree_insert_node(&(mmap->length_tree),&(group->rb_node));
	}
	free_region=omm_alloc(_mmap_free_region_allocator);
	free_region->rb_node.key=address;
	free_region->prev=NULL;
	free_region->next=group->head;
	free_region->group=group;
	if (group->head){
		group->head->prev=free_region;
	}
	group->head=free_region;
	rb_tree_insert_node(&(mmap->free_address_tree),&(free_region->rb_node));
}



static void _dealloc_region(mmap_t* mmap,mmap_region_t* region){
	rb_tree_remove_node(&(mmap->address_tree),&(region->rb_node));
	u64 guard_page_size=((region->flags&MMAP_REGION_FLAG_STACK)?MMAP_STACK_GUARD_PAGE_COUNT<<PAGE_SIZE_SHIFT:0);
	for (u64 address=0;address<region->length;address+=PAGE_SIZE){
		u64 entry=vmm_unmap_page(mmap->pagemap,region->rb_node.key+address)&VMM_PAGE_ADDRESS_MASK;
		if (entry){
			pf_invalidate_tlb_entry(region->rb_node.key+address);
		}
		if ((entry&VMM_PAGE_ADDRESS_MASK)&&!(region->flags&MMAP_REGION_FLAG_EXTERNAL)){
			if (region->file&&!(region->flags&MMAP_REGION_FLAG_NO_WRITEBACK)){
				panic("mmap_dealloc: file-backed memory region writeback");
			}
			pmm_dealloc(entry&VMM_PAGE_ADDRESS_MASK,1,_mmap_pmm_counter);
		}
	}
	_push_free_region(mmap,region->rb_node.key-guard_page_size,region->length+guard_page_size);
	omm_dealloc(_mmap_region_allocator,region);
}



KERNEL_EARLY_EARLY_INIT(){
	LOG("Initializing mmap allocator...");
	_mmap_pmm_counter=pmm_alloc_counter("mmap");
	_mmap_allocator=omm_init("mmap",sizeof(mmap_t),8,4,pmm_alloc_counter("omm_mmap"));
	spinlock_init(&(_mmap_allocator->lock));
	_mmap_region_allocator=omm_init("mmap_region",sizeof(mmap_region_t),8,4,pmm_alloc_counter("omm_mmap_region"));
	spinlock_init(&(_mmap_region_allocator->lock));
	_mmap_length_group_allocator=omm_init("mmap_length_group",sizeof(mmap_length_group_t),8,4,pmm_alloc_counter("omm_mmap_length_group"));
	spinlock_init(&(_mmap_length_group_allocator->lock));
	_mmap_free_region_allocator=omm_init("mmap_free_region",sizeof(mmap_free_region_t),8,4,pmm_alloc_counter("omm_mmap_free_region"));
	spinlock_init(&(_mmap_free_region_allocator->lock));
}



mmap_t* mmap_init(vmm_pagemap_t* pagemap,u64 bottom_address,u64 top_address){
	mmap_t* out=omm_alloc(_mmap_allocator);
	spinlock_init(&(out->lock));
	out->pagemap=pagemap;
	out->bottom_address=bottom_address;
	out->break_address=bottom_address;
	out->heap_address=top_address;
	out->top_address=top_address;
	rb_tree_init(&(out->address_tree));
	rb_tree_init(&(out->length_tree));
	rb_tree_init(&(out->free_address_tree));
	return out;
}



void mmap_deinit(mmap_t* mmap){
	ERROR("mmap_deinit");
	omm_dealloc(_mmap_allocator,mmap);
}



KERNEL_PUBLIC mmap_region_t* mmap_alloc(mmap_t* mmap,u64 address,u64 length,u32 flags,vfs_node_t* file){
	if ((address|length)&(PAGE_SIZE-1)){
		return NULL;
	}
	if (!length&&file){
		length=pmm_align_up_address(vfs_node_resize(file,0,VFS_NODE_FLAG_RESIZE_RELATIVE));
	}
	if (!length||(length>>47)){
		return NULL;
	}
	u64 guard_page_size=((flags&MMAP_REGION_FLAG_STACK)?MMAP_STACK_GUARD_PAGE_COUNT<<PAGE_SIZE_SHIFT:0);
	length+=guard_page_size;
	spinlock_acquire_exclusive(&(mmap->lock));
	if (!address){
		mmap_length_group_t* group=(void*)rb_tree_lookup_increasing_node(&(mmap->length_tree),length);
		if (group){
			mmap_free_region_t* free_region=group->head;
			u64 free_region_length=group->rb_node.key;
			address=free_region->rb_node.key;
			_pop_free_region(mmap,free_region);
			if (free_region_length>length){
				_push_free_region(mmap,address+length,free_region_length-length);
			}
		}
		else{
			if (mmap->break_address+length>mmap->heap_address){
				spinlock_release_exclusive(&(mmap->lock));
				return NULL;
			}
			mmap->heap_address-=length;
			address=mmap->heap_address;
		}
	}
	mmap_region_t* out=omm_alloc(_mmap_region_allocator);
	out->rb_node.key=address+guard_page_size;
	out->length=length-guard_page_size;
	out->flags=flags;
	out->file=file;
	rb_tree_insert_node(&(mmap->address_tree),&(out->rb_node));
	spinlock_release_exclusive(&(mmap->lock));
	if (flags&MMAP_REGION_FLAG_COMMIT){
		for (u64 offset=address+guard_page_size;offset<address+length;offset+=PAGE_SIZE){
			mmap_handle_pf(mmap,offset);
		}
	}
	return out;
}



KERNEL_PUBLIC _Bool mmap_dealloc(mmap_t* mmap,u64 address,u64 length){
	spinlock_acquire_exclusive(&(mmap->lock));
	mmap_region_t* region=(void*)rb_tree_lookup_decreasing_node(&(mmap->address_tree),address);
	if (!region||region->rb_node.key+region->length<=address){
		spinlock_release_exclusive(&(mmap->lock));
		return 0;
	}
	if (!length){
		length=region->rb_node.key+region->length-address;
	}
	if (region->rb_node.key!=address||region->length!=length){
		panic("mmap_dealloc: partial dealloc");
	}
	_dealloc_region(mmap,region);
	spinlock_release_exclusive(&(mmap->lock));
	return 1;
}



KERNEL_PUBLIC void mmap_dealloc_region(mmap_t* mmap,mmap_region_t* region){
	spinlock_acquire_exclusive(&(mmap->lock));
	_dealloc_region(mmap,region);
	spinlock_release_exclusive(&(mmap->lock));
}



KERNEL_PUBLIC mmap_region_t* mmap_lookup(mmap_t* mmap,u64 address){
	if (!mmap){
		return NULL;
	}
	spinlock_acquire_exclusive(&(mmap->lock));
	mmap_region_t* out=(void*)rb_tree_lookup_decreasing_node(&(mmap->address_tree),address);
	if (out&&out->rb_node.key+out->length<=address){
		out=NULL;
	}
	spinlock_release_exclusive(&(mmap->lock));
	return out;
}



KERNEL_PUBLIC mmap_region_t* mmap_map_to_kernel(mmap_t* mmap,u64 address,u64 length){
	mmap_region_t* out=mmap_alloc(process_kernel->mmap,0,length,MMAP_REGION_FLAG_EXTERNAL|MMAP_REGION_FLAG_VMM_WRITE,NULL);
	for (u64 offset=address;offset<address+length;offset+=PAGE_SIZE){
		u64 physical_address=vmm_virtual_to_physical(mmap->pagemap,offset);
		if (!physical_address){
			physical_address=mmap_handle_pf(mmap,offset);
			if (!physical_address){
				panic("mmap_map_to_kernel: invalid address");
			}
		}
		vmm_map_page(&(process_kernel->pagemap),physical_address,out->rb_node.key+offset-address,VMM_PAGE_FLAG_NOEXECUTE|VMM_PAGE_FLAG_READWRITE|VMM_PAGE_FLAG_PRESENT);
	}
	return out;
}



u64 mmap_handle_pf(mmap_t* mmap,u64 address){
	if (!mmap){
		return 0;
	}
	spinlock_acquire_exclusive(&(mmap->lock));
	mmap_region_t* region=(void*)rb_tree_lookup_decreasing_node(&(mmap->address_tree),address);
	if (!region||region->rb_node.key+region->length<=address){
		spinlock_release_exclusive(&(mmap->lock));
		return 0;
	}
	u64 out=pmm_alloc(1,_mmap_pmm_counter,0);
	if (region->file){
		vfs_node_read(region->file,address-region->rb_node.key,(void*)(out+VMM_HIGHER_HALF_ADDRESS_OFFSET),PAGE_SIZE,0);
	}
	u64 flags=VMM_PAGE_FLAG_PRESENT;
	if (region->flags&MMAP_REGION_FLAG_VMM_USER){
		flags|=VMM_PAGE_FLAG_USER;
	}
	if (region->flags&MMAP_REGION_FLAG_VMM_WRITE){
		flags|=VMM_PAGE_FLAG_READWRITE;
	}
	if (!(region->flags&MMAP_REGION_FLAG_VMM_EXEC)){
		flags|=VMM_PAGE_FLAG_NOEXECUTE;
	}
	vmm_map_page(mmap->pagemap,out,address,flags);
	spinlock_release_exclusive(&(mmap->lock));
	return out;
}



error_t syscall_memory_map(u64 size,u64 flags,handle_id_t fd){
	u64 mmap_flags=MMAP_REGION_FLAG_VMM_USER;
	vfs_node_t* file=NULL;
	if (flags&USER_MEMORY_FLAG_WRITE){
		mmap_flags|=MMAP_REGION_FLAG_VMM_WRITE;
	}
	if (flags&USER_MEMORY_FLAG_EXEC){
		mmap_flags|=MMAP_REGION_FLAG_VMM_EXEC;
	}
	if (flags&USER_MEMORY_FLAG_FILE){
		file=fd_get_node(fd);
		if (!file){
			return ERROR_INVALID_HANDLE;
		}
	}
	if (flags&USER_MEMORY_FLAG_NOWRITEBACK){
		mmap_flags|=MMAP_REGION_FLAG_NO_WRITEBACK;
	}
	if (flags&USER_MEMORY_FLAG_STACK){
		mmap_flags|=MMAP_REGION_FLAG_STACK;
	}
	mmap_region_t* out=mmap_alloc(THREAD_DATA->process->mmap,0,pmm_align_up_address(size),mmap_flags,file);
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
	mmap_t* mmap=THREAD_DATA->process->mmap;
	for (u64 offset=address;offset<address+size;offset+=PAGE_SIZE){
		if (!vmm_virtual_to_physical(mmap->pagemap,offset)&&!mmap_handle_pf(mmap,offset)){
			return ERROR_INVALID_ARGUMENT(0);
		}
		vmm_adjust_flags(mmap->pagemap,offset,vmm_set_flags,VMM_PAGE_FLAG_NOEXECUTE|VMM_PAGE_FLAG_READWRITE,1,1);
	}
	return ERROR_OK;
}



error_t syscall_memory_unmap(u64 address,u64 size){
	return (mmap_dealloc(THREAD_DATA->process->mmap,pmm_align_down_address(address),pmm_align_up_address(size+(address&(PAGE_SIZE-1))))?ERROR_OK:ERROR_INVALID_ARGUMENT(0));
}
