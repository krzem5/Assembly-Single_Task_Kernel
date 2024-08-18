#include <kernel/error/error.h>
#include <kernel/exception/exception.h>
#include <kernel/fd/fd.h>
#include <kernel/format/format.h>
#include <kernel/isr/pf.h>
#include <kernel/lock/rwlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/mmap/mmap.h>
#include <kernel/mp/process.h>
#include <kernel/mp/thread.h>
#include <kernel/scheduler/scheduler.h>
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



static pmm_counter_descriptor_t* KERNEL_INIT_WRITE _mmap_pmm_counter=NULL;
static omm_allocator_t* KERNEL_INIT_WRITE _mmap_allocator=NULL;
static omm_allocator_t* KERNEL_INIT_WRITE _mmap_region_allocator=NULL;
static omm_allocator_t* KERNEL_INIT_WRITE _mmap_length_group_allocator=NULL;
static omm_allocator_t* KERNEL_INIT_WRITE _mmap_free_region_allocator=NULL;



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
		// ERROR("%v",free_region->group->rb_node.key);
		// length+=free_region->group->rb_node.key;
		// _pop_free_region(mmap,free_region);
	}
	free_region=(void*)rb_tree_lookup_decreasing_node(&(mmap->free_address_tree),address);
	if (free_region&&free_region->rb_node.key+free_region->group->rb_node.key==address){
		address-=free_region->group->rb_node.key;
		length+=free_region->group->rb_node.key;
		_pop_free_region(mmap,free_region);
	}
	if (address==mmap->heap_address){
		mmap->heap_address+=length;
		return;
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



static void _unmap_address(mmap_t* mmap,mmap_region_t* region,u64 address){
	u64 entry=vmm_unmap_page(mmap->pagemap,address)&VMM_PAGE_ADDRESS_MASK;
	if (!entry){
		return;
	}
	pf_invalidate_tlb_entry(address);
	if (!region||!(region->flags&MMAP_REGION_FLAG_EXTERNAL)){
		if (region&&region->file&&!(region->flags&MMAP_REGION_FLAG_NO_WRITEBACK)){
			panic("mmap_dealloc: file-backed memory region writeback");
		}
		pmm_dealloc(entry,1,_mmap_pmm_counter);
	}
}



static void _dealloc_region(mmap_t* mmap,mmap_region_t* region,bool push_free_region){
	rb_tree_remove_node(&(mmap->address_tree),&(region->rb_node));
	for (u64 address=0;address<region->length;address+=PAGE_SIZE){
		_unmap_address(mmap,region,region->rb_node.key+address);
	}
	if (region->file){
		vfs_node_unref(region->file);
	}
	if (push_free_region){
		u64 guard_page_size=((region->flags&MMAP_REGION_FLAG_STACK)?MMAP_STACK_GUARD_PAGE_COUNT<<PAGE_SIZE_SHIFT:0);
		_push_free_region(mmap,region->rb_node.key-guard_page_size,region->length+guard_page_size);
	}
	omm_dealloc(_mmap_region_allocator,region);
}



KERNEL_EARLY_EARLY_INIT(){
	_mmap_pmm_counter=pmm_alloc_counter("kernel.mmap");
	_mmap_allocator=omm_init("kernel.mmap",sizeof(mmap_t),8,4);
	rwlock_init(&(_mmap_allocator->lock));
	_mmap_region_allocator=omm_init("kernel.mmap.region",sizeof(mmap_region_t),8,4);
	rwlock_init(&(_mmap_region_allocator->lock));
	_mmap_length_group_allocator=omm_init("kernel.mmap.length_group",sizeof(mmap_length_group_t),8,4);
	rwlock_init(&(_mmap_length_group_allocator->lock));
	_mmap_free_region_allocator=omm_init("kernel.mmap.free_region",sizeof(mmap_free_region_t),8,4);
	rwlock_init(&(_mmap_free_region_allocator->lock));
}



mmap_t* mmap_init(vmm_pagemap_t* pagemap,u64 bottom_address,u64 top_address){
	mmap_t* out=omm_alloc(_mmap_allocator);
	rwlock_init(&(out->lock));
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
	while (1){
		mmap_region_t* region=(void*)rb_tree_lookup_increasing_node(&(mmap->address_tree),0);
		if (!region){
			break;
		}
		_dealloc_region(mmap,region,0);
	}
	while (1){
		mmap_length_group_t* group=(void*)rb_tree_lookup_increasing_node(&(mmap->length_tree),0);
		if (!group){
			break;
		}
		rb_tree_remove_node(&(mmap->length_tree),&(group->rb_node));
		for (mmap_free_region_t* free_region=group->head;free_region;){
			mmap_free_region_t* next=free_region->next;
			omm_dealloc(_mmap_free_region_allocator,free_region);
			free_region=next;
		}
		omm_dealloc(_mmap_length_group_allocator,group);
	}
	omm_dealloc(_mmap_allocator,mmap);
}



KERNEL_PUBLIC KERNEL_AWAITS mmap_region_t* mmap_alloc(mmap_t* mmap,u64 address,u64 length,u32 flags,vfs_node_t* file){
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
	rwlock_acquire_write(&(mmap->lock));
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
				rwlock_release_write(&(mmap->lock));
				return NULL;
			}
			mmap->heap_address-=length;
			address=mmap->heap_address;
		}
	}
	if (file){
		vfs_node_ref(file);
	}
	mmap_region_t* out=omm_alloc(_mmap_region_allocator);
	out->rb_node.key=address+guard_page_size;
	out->length=length-guard_page_size;
	out->flags=flags;
	out->file=file;
	rb_tree_insert_node(&(mmap->address_tree),&(out->rb_node));
	rwlock_release_write(&(mmap->lock));
	if (flags&MMAP_REGION_FLAG_COMMIT){
		exception_unwind_push(mmap,out){
			mmap_dealloc_region(EXCEPTION_UNWIND_ARG(0),EXCEPTION_UNWIND_ARG(1));
		}
		for (u64 offset=address+guard_page_size;offset<address+length;offset+=PAGE_SIZE){
			mmap_handle_pf(mmap,offset,NULL);
		}
		exception_unwind_pop();
	}
	return out;
}



KERNEL_PUBLIC bool mmap_dealloc(mmap_t* mmap,u64 address,u64 length){
	rwlock_acquire_write(&(mmap->lock));
	mmap_region_t* region=(void*)rb_tree_lookup_decreasing_node(&(mmap->address_tree),address);
	if (!region||region->rb_node.key+region->length<=address){
		rwlock_release_write(&(mmap->lock));
		return 0;
	}
	if (!length){
		length=region->rb_node.key+region->length-address;
	}
	if (region->rb_node.key!=address||region->length!=length){
		panic("mmap_dealloc: partial dealloc");
	}
	_dealloc_region(mmap,region,1);
	rwlock_release_write(&(mmap->lock));
	return 1;
}



KERNEL_PUBLIC void mmap_dealloc_region(mmap_t* mmap,mmap_region_t* region){
	rwlock_acquire_write(&(mmap->lock));
	_dealloc_region(mmap,region,1);
	rwlock_release_write(&(mmap->lock));
}



KERNEL_PUBLIC mmap_region_t* mmap_lookup(mmap_t* mmap,u64 address){
	if (!mmap){
		return NULL;
	}
	rwlock_acquire_write(&(mmap->lock));
	mmap_region_t* out=(void*)rb_tree_lookup_decreasing_node(&(mmap->address_tree),address);
	if (out&&out->rb_node.key+out->length<=address){
		out=NULL;
	}
	rwlock_release_write(&(mmap->lock));
	return out;
}



KERNEL_PUBLIC KERNEL_AWAITS mmap_region_t* mmap_map_to_kernel(mmap_t* mmap,u64 address,u64 length){
	mmap_region_t* out=mmap_alloc(process_kernel->mmap,0,length,MMAP_REGION_FLAG_EXTERNAL|MMAP_REGION_FLAG_VMM_WRITE,NULL);
	for (u64 offset=address;offset<address+length;offset+=PAGE_SIZE){
		u64 physical_address=vmm_virtual_to_physical(mmap->pagemap,offset);
		if (!physical_address){
			physical_address=mmap_handle_pf(mmap,offset,NULL);
			if (!physical_address){
				ERROR("mmap_map_to_kernel: address '%p' is not mapped in the process address space",offset);
				panic("mmap_map_to_kernel: invalid address");
			}
		}
		vmm_map_page(&(process_kernel->pagemap),physical_address,out->rb_node.key+offset-address,VMM_PAGE_FLAG_NOEXECUTE|VMM_PAGE_FLAG_READWRITE|VMM_PAGE_FLAG_PRESENT);
	}
	return out;
}



KERNEL_NO_AWAITS u64 mmap_handle_pf(mmap_t* mmap,u64 address,void* isr_state){
	if (!mmap){
		return 0;
	}
	rwlock_acquire_write(&(mmap->lock));
	mmap_region_t* region=(void*)rb_tree_lookup_decreasing_node(&(mmap->address_tree),address);
	if (!region||(region->flags&MMAP_REGION_FLAG_NO_ALLOC)||region->rb_node.key+region->length<=address){
		rwlock_release_write(&(mmap->lock));
		return 0;
	}
	u64 out=pmm_alloc(1,_mmap_pmm_counter,0);
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
	pf_invalidate_tlb_entry(address);
	rwlock_release_write(&(mmap->lock));
	if (!region->file){
		return out;
	}
	if (!isr_state){
		vfs_node_read(region->file,address-region->rb_node.key,(void*)(out+VMM_HIGHER_HALF_ADDRESS_OFFSET),PAGE_SIZE,0);
		return out;
	}
	if (!CPU_HEADER_DATA->current_thread){
		panic("mmap_handle_pf: scheduler file-backed memory page fault");
	}
	char buffer[256];
	format_string(buffer,sizeof(buffer),"kernel.pf.file.read/%s",THREAD_DATA->name->data);
	thread_t* thread=thread_create_kernel_thread(process_kernel,buffer,vfs_node_read,5,region->file,address-region->rb_node.key,(void*)(out+VMM_HIGHER_HALF_ADDRESS_OFFSET),PAGE_SIZE,0);
	scheduler_irq_return_after_thread(isr_state,thread);
	handle_release(&(thread->handle));
	return out;
}



void mmap_unmap_address(mmap_t* mmap,u64 address){
	rwlock_acquire_write(&(mmap->lock));
	mmap_region_t* region=(void*)rb_tree_lookup_decreasing_node(&(mmap->address_tree),address);
	if (region&&region->rb_node.key+region->length<=address){
		region=NULL;
	}
	_unmap_address(mmap,region,address);
	rwlock_release_write(&(mmap->lock));
}



KERNEL_NO_AWAITS error_t syscall_memory_map(u64 size,u64 flags,handle_id_t fd){
	u64 mmap_flags=MMAP_REGION_FLAG_VMM_USER;
	vfs_node_t* file=NULL;
	if (flags&USER_MEMORY_FLAG_WRITE){
		mmap_flags|=MMAP_REGION_FLAG_VMM_WRITE;
	}
	if (flags&USER_MEMORY_FLAG_EXEC){
		mmap_flags|=MMAP_REGION_FLAG_VMM_EXEC;
	}
	if (flags&USER_MEMORY_FLAG_FILE){
		u64 acl;
		file=fd_get_node(fd,&acl);
		if (!file){
			return ERROR_INVALID_HANDLE;
		}
		if (!(acl&FD_ACL_FLAG_IO)){
			return ERROR_DENIED;
		}
	}
	if (flags&USER_MEMORY_FLAG_NOWRITEBACK){
		mmap_flags|=MMAP_REGION_FLAG_NO_WRITEBACK;
	}
	if (flags&USER_MEMORY_FLAG_STACK){
		mmap_flags|=MMAP_REGION_FLAG_STACK;
	}
	mmap_region_t* out=mmap_alloc(THREAD_DATA->process->mmap,0,pmm_align_up_address(size),mmap_flags,file);
	if (file){
		vfs_node_unref(file);
	}
	return (out?out->rb_node.key:ERROR_NO_MEMORY);
}



KERNEL_NO_AWAITS error_t syscall_memory_change_flags(u64 address,u64 size,u64 flags){
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
		if (!vmm_virtual_to_physical(mmap->pagemap,offset)&&!mmap_handle_pf(mmap,offset,NULL)){
			return ERROR_INVALID_ARGUMENT(0);
		}
		vmm_adjust_flags(mmap->pagemap,offset,vmm_set_flags,VMM_PAGE_FLAG_NOEXECUTE|VMM_PAGE_FLAG_READWRITE,1,1);
	}
	return ERROR_OK;
}



error_t syscall_memory_get_size(u64 address){
	mmap_region_t* region=mmap_lookup(THREAD_DATA->process->mmap,address);
	return (region?region->length:ERROR_INVALID_ARGUMENT(0));
}



error_t syscall_memory_unmap(u64 address,u64 size){
	return (mmap_dealloc(THREAD_DATA->process->mmap,pmm_align_down_address(address),pmm_align_up_address(size+(address&(PAGE_SIZE-1))))?ERROR_OK:ERROR_INVALID_ARGUMENT(0));
}
