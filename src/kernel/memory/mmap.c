#include <kernel/fd/fd.h>
#include <kernel/isr/pf.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/mmap.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/mp/thread.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/vfs/node.h>
#define KERNEL_LOG_NAME "mmap"



#define USER_MEMORY_FLAG_READ 1
#define USER_MEMORY_FLAG_WRITE 2
#define USER_MEMORY_FLAG_EXEC 4
#define USER_MEMORY_FLAG_FILE 8
#define USER_MEMORY_FLAG_NOWRITEBACK 16



static pmm_counter_descriptor_t* _mmap_file_pmm_counter=NULL;
static pmm_counter_descriptor_t* _mmap_generic_pmm_counter=NULL;
static pmm_counter_descriptor_t* _mmap_user_data_pmm_counter=NULL;
static omm_allocator_t* _mmap_region_allocator=NULL;
static omm_allocator_t* _mmap_length_group_allocator=NULL;



static void _add_region_to_length_tree(mmap_t* mmap,mmap_region_t* region){
	KERNEL_ASSERT(spinlock_is_held(&(mmap->lock)));
	KERNEL_ASSERT(!region->flags);
	mmap_length_group_t* length_group=(void*)rb_tree_lookup_node(&(mmap->length_tree),region->length);
	if (length_group){
		region->group_prev=NULL;
		region->group_next=length_group->head;
		if (length_group->head){
			length_group->head->group_prev=region;
		}
		length_group->head=region;
		return;
	}
	length_group=omm_alloc(_mmap_length_group_allocator);
	length_group->rb_node.key=region->length;
	length_group->head=region;
	region->group=length_group;
	region->group_prev=NULL;
	region->group_next=NULL;
	rb_tree_insert_node(&(mmap->length_tree),&(length_group->rb_node));
}



static void _remove_region_from_length_tree(mmap_t* mmap,mmap_region_t* region){
	KERNEL_ASSERT(spinlock_is_held(&(mmap->lock)));
	mmap_length_group_t* length_group=region->group;
	KERNEL_ASSERT(length_group->rb_node.key==region->length);
	if (region->group_prev){
		region->group_prev->group_next=region->group_next;
	}
	else{
		length_group->head=region->group_next;
	}
	if (region->group_next){
		region->group_next->group_prev=region->group_prev;
	}
	if (!length_group->head){
		rb_tree_remove_node(&(mmap->length_tree),&(length_group->rb_node));
		omm_dealloc(_mmap_length_group_allocator,length_group);
	}
}



static void _delete_pagemap_pages(mmap_t* mmap,mmap_region_t* region){
	KERNEL_ASSERT(spinlock_is_held(&(mmap->lock)));
	for (u64 i=0;i<region->length;i+=PAGE_SIZE){
		u64 entry=vmm_unmap_page(mmap->pagemap,region->rb_node.key+i)&VMM_PAGE_ADDRESS_MASK;
		if (entry){
			pf_invalidate_tlb_entry(region->rb_node.key+i);
		}
		if (entry&VMM_PAGE_ADDRESS_MASK){
			if (region->file&&!(region->flags&MMAP_REGION_FLAG_NO_FILE_WRITEBACK)){
				panic("mmap_dealloc: file-backed memory region writeback");
			}
			pmm_dealloc(entry&VMM_PAGE_ADDRESS_MASK,1,region->pmm_counter);
		}
	}
}



static _Bool _dealloc_region(mmap_t* mmap,mmap_region_t* region){
	KERNEL_ASSERT(spinlock_is_held(&(mmap->lock)));
	if (!region->flags){
		spinlock_release_exclusive(&(mmap->lock));
		return 0;
	}
	_delete_pagemap_pages(mmap,region);
	region->flags=0;
	spinlock_release_exclusive(&(mmap->lock));return 1;
	if (region->prev&&!region->prev->flags){
		mmap_region_t* prev_region=region->prev;
		rb_tree_remove_node(&(mmap->offset_tree),&(region->rb_node));
		_remove_region_from_length_tree(mmap,prev_region);
		prev_region->length+=region->length;
		prev_region->next=region->next;
		if (prev_region->next){
			prev_region->next->prev=prev_region;
		}
		omm_dealloc(_mmap_region_allocator,region);
		region=prev_region;
	}
	if (region->next&&!region->next->flags){
		mmap_region_t* next_region=region->next;
		rb_tree_remove_node(&(mmap->offset_tree),&(next_region->rb_node));
		_remove_region_from_length_tree(mmap,next_region);
		region->length+=next_region->length;
		region->next=next_region->next;
		if (region->next){
			region->next->prev=region;
		}
		omm_dealloc(_mmap_region_allocator,next_region);
	}
	_add_region_to_length_tree(mmap,region);
	KERNEL_ASSERT_BLOCK({
		for (rb_tree_node_t* rb_node=rb_tree_iter_start(&(mmap->offset_tree));rb_node;rb_node=rb_tree_iter_next(&(mmap->offset_tree),rb_node)){
			mmap_region_t* region=(void*)rb_node;
			KERNEL_ASSERT(region->prev==(mmap_region_t*)rb_tree_lookup_decreasing_node(&(mmap->offset_tree),rb_node->key-1));
			KERNEL_ASSERT(region->next==(mmap_region_t*)rb_tree_lookup_increasing_node(&(mmap->offset_tree),rb_node->key+1));
			KERNEL_ASSERT(!region->next||rb_node->key+region->length==region->next->rb_node.key);
			KERNEL_ASSERT(!region->next||region->flags||region->next->flags);
		}
	});
	spinlock_release_exclusive(&(mmap->lock));
	return 1;
}



KERNEL_EARLY_EARLY_INIT(){
	LOG("Initializing mmap allocator...");
	_mmap_region_allocator=omm_init("mmap_region",sizeof(mmap_region_t),8,4,pmm_alloc_counter("omm_mmap_region"));
	spinlock_init(&(_mmap_region_allocator->lock));
	_mmap_length_group_allocator=omm_init("mmap_length_group",sizeof(mmap_length_group_t),8,4,pmm_alloc_counter("omm_mmap_length_group"));
	spinlock_init(&(_mmap_length_group_allocator->lock));
	_mmap_file_pmm_counter=pmm_alloc_counter("mmap_file");
	_mmap_generic_pmm_counter=pmm_alloc_counter("mmap_generic");
	_mmap_user_data_pmm_counter=pmm_alloc_counter("mmap_user_data");
}



KERNEL_PUBLIC void mmap_init(vmm_pagemap_t* pagemap,u64 low,u64 high,mmap_t* out){
	KERNEL_ASSERT(pagemap);
	KERNEL_ASSERT(out);
	out->pagemap=pagemap;
	spinlock_init(&(out->lock));
	spinlock_acquire_exclusive(&(out->lock));
	rb_tree_init(&(out->offset_tree));
	rb_tree_init(&(out->length_tree));
	mmap_region_t* region=omm_alloc(_mmap_region_allocator);
	region->flags=0;
	region->rb_node.key=low;
	region->length=high-low;
	region->prev=NULL;
	region->next=NULL;
	rb_tree_insert_node(&(out->offset_tree),&(region->rb_node));
	_add_region_to_length_tree(out,region);
	spinlock_release_exclusive(&(out->lock));
}



KERNEL_PUBLIC void mmap_deinit(mmap_t* mmap){
	KERNEL_ASSERT(mmap);
	spinlock_acquire_exclusive(&(mmap->lock));
	for (rb_tree_node_t* rb_node=rb_tree_iter_start(&(mmap->offset_tree));rb_node;){
		mmap_region_t* region=(void*)rb_node;
		rb_node=rb_tree_iter_next(&(mmap->offset_tree),rb_node);
		if (region->flags){
			_delete_pagemap_pages(mmap,region);
		}
		omm_dealloc(_mmap_region_allocator,region);
	}
	for (rb_tree_node_t* rb_node=rb_tree_iter_start(&(mmap->length_tree));rb_node;){
		mmap_length_group_t* length_group=(void*)rb_node;
		rb_node=rb_tree_iter_next(&(mmap->length_tree),rb_node);
		omm_dealloc(_mmap_length_group_allocator,length_group);
	}
	rb_tree_init(&(mmap->offset_tree));
	rb_tree_init(&(mmap->length_tree));
}



KERNEL_PUBLIC mmap_region_t* mmap_alloc(mmap_t* mmap,u64 address,u64 length,pmm_counter_descriptor_t* pmm_counter,u64 flags,vfs_node_t* file){
	KERNEL_ASSERT(mmap);
	if ((address|length)&(PAGE_SIZE-1)){
		panic("mmap_alloc: unaligned arguments");
	}
	if (!pmm_counter){
		pmm_counter=(file?_mmap_file_pmm_counter:_mmap_generic_pmm_counter);
	}
	if (!length&&file){
		length=pmm_align_up_address(vfs_node_resize(file,0,VFS_NODE_FLAG_RESIZE_RELATIVE));
	}
	if (!length){
		return NULL;
	}
	spinlock_acquire_exclusive(&(mmap->lock));
	mmap_region_t* region;
	if (address){
		region=(void*)rb_tree_lookup_decreasing_node(&(mmap->offset_tree),address);
		if (!region){
			spinlock_release_exclusive(&(mmap->lock));
			return NULL;
		}
	}
	else{
		mmap_length_group_t* length_group=(void*)rb_tree_lookup_increasing_node(&(mmap->length_tree),address);
		if (!length_group){
			spinlock_release_exclusive(&(mmap->lock));
			return NULL;
		}
		region=length_group->head;
		length_group->head=region->group_next;
		if (region->group_next){
			region->group_next->group_prev=NULL;
		}
	}
	if (!address){
		address=region->rb_node.key;
	}
	else if (region->flags||address-region->rb_node.key+length>region->length){
		spinlock_release_exclusive(&(mmap->lock));
		return NULL;
	}
	_remove_region_from_length_tree(mmap,region);
	if (address!=region->rb_node.key){
		mmap_region_t* new_region=omm_alloc(_mmap_region_allocator);
		new_region->rb_node.key=address;
		new_region->length=region->rb_node.key+region->length-address;
		new_region->prev=region;
		new_region->next=region->next;
		if (region->next){
			region->next->prev=new_region;
		}
		region->length=address-region->rb_node.key;
		region->next=new_region;
		_add_region_to_length_tree(mmap,region);
		rb_tree_insert_node(&(mmap->offset_tree),&(new_region->rb_node));
		region=new_region;
	}
	if (region->length>length){
		mmap_region_t* new_region=omm_alloc(_mmap_region_allocator);
		new_region->flags=0;
		new_region->rb_node.key=address+length;
		new_region->length=region->length-length;
		new_region->prev=region;
		new_region->next=region->next;
		if (region->next){
			region->next->prev=new_region;
		}
		region->length=length;
		region->next=new_region;
		rb_tree_insert_node(&(mmap->offset_tree),&(new_region->rb_node));
		_add_region_to_length_tree(mmap,new_region);
	}
	region->flags=flags|MMAP_REGION_FLAG_USED;
	region->pmm_counter=pmm_counter;
	region->file=file;
	spinlock_release_exclusive(&(mmap->lock));
	if (flags&MMAP_REGION_FLAG_COMMIT){
		if (file){
			panic("mmap_alloc: both file and MMAP_REGION_FLAG_COMMIT specfied");
		}
		u64 vmm_flags=mmap_get_vmm_flags(region);
		for (u64 i=0;i<region->length;i+=PAGE_SIZE){
			vmm_map_page(mmap->pagemap,pmm_alloc(1,pmm_counter,0),address+i,vmm_flags);
		}
	}
	return region;
}



KERNEL_PUBLIC _Bool mmap_dealloc(mmap_t* mmap,u64 address,u64 length){
	KERNEL_ASSERT(mmap);
	if ((address|length)&(PAGE_SIZE-1)){
		panic("mmap_dealloc: unaligned arguments");
	}
	spinlock_acquire_exclusive(&(mmap->lock));
	mmap_region_t* region=(mmap_region_t*)rb_tree_lookup_decreasing_node(&(mmap->offset_tree),address);
	if (!region){
		spinlock_release_exclusive(&(mmap->lock));
		return 0;
	}
	if (region->rb_node.key!=address||(length&&region->length!=length)){
		panic("mmap_dealloc: partial release");
	}
	return _dealloc_region(mmap,region);
}



KERNEL_PUBLIC _Bool mmap_dealloc_region(mmap_t* mmap,mmap_region_t* region){
	KERNEL_ASSERT(mmap);
	KERNEL_ASSERT(region);
	spinlock_acquire_exclusive(&(mmap->lock));
	return _dealloc_region(mmap,region);
}



KERNEL_PUBLIC _Bool mmap_change_flags(mmap_t* mmap,u64 address,u64 length,u64 vmm_set_flags,u64 vmm_clear_flags){
	KERNEL_ASSERT(mmap);
	if ((address|length)&(PAGE_SIZE-1)){
		panic("mmap_change_flags: unaligned arguments");
	}
	spinlock_acquire_exclusive(&(mmap->lock));
	mmap_region_t* region=(mmap_region_t*)rb_tree_lookup_decreasing_node(&(mmap->offset_tree),address);
	if (!region){
		spinlock_release_exclusive(&(mmap->lock));
		return 0;
	}
	if (length>region->length){
		panic("mmap_change_flags: length too large for region");
	}
	for (u64 i=address;i<address+length;i+=PAGE_SIZE){
		if (!vmm_virtual_to_physical(mmap->pagemap,i)){
			if (region->file){
				panic("mmap_change_flags: unable to change flags on nonaccessed file mapping");
			}
			vmm_map_page(mmap->pagemap,pmm_alloc(1,region->pmm_counter,0),i,mmap_get_vmm_flags(region));
		}
		vmm_adjust_flags(mmap->pagemap,i,vmm_set_flags,vmm_clear_flags,1,1);
	}
	spinlock_release_exclusive(&(mmap->lock));
	return 0;
}



KERNEL_PUBLIC _Bool mmap_set_memory(mmap_t* mmap,mmap_region_t* region,u64 offset,const void* data,u64 length){
	KERNEL_ASSERT(mmap);
	KERNEL_ASSERT(region);
	KERNEL_ASSERT(!length||data);
	spinlock_acquire_shared(&(mmap->lock));
	if (!(region->flags&MMAP_REGION_FLAG_COMMIT)){
		panic("mmap_set_memory: MMAP_REGION_FLAG_COMMIT flag missing");
	}
	if (length>region->length){
		panic("mmap_set_memory: length too large for region");
	}
	u64 end=offset+length;
	for (u64 i=pmm_align_down_address(offset);i<end;i+=PAGE_SIZE){
		u64 physical_address=vmm_virtual_to_physical(mmap->pagemap,region->rb_node.key+i);
		if (!physical_address){
			panic("mmap_set_memory: wrong memory mapping");
		}
		offset&=PAGE_SIZE-1;
		u64 chunk_size=(length>PAGE_SIZE-offset?PAGE_SIZE-offset:length);
		memcpy((void*)(physical_address+offset+VMM_HIGHER_HALF_ADDRESS_OFFSET),data,chunk_size);
		offset=0;
		data+=chunk_size;
		length-=chunk_size;
	}
	spinlock_release_shared(&(mmap->lock));
	return 1;
}



KERNEL_PUBLIC mmap_region_t* mmap_lookup(mmap_t* mmap,u64 address){
	KERNEL_ASSERT(mmap);
	spinlock_acquire_shared(&(mmap->lock));
	mmap_region_t* out=(void*)rb_tree_lookup_decreasing_node(&(mmap->offset_tree),address);
	spinlock_release_shared(&(mmap->lock));
	return (out&&out->flags?out:NULL);
}



KERNEL_PUBLIC u64 mmap_get_vmm_flags(mmap_region_t* region){
	KERNEL_ASSERT(region);
	u64 out=VMM_PAGE_FLAG_PRESENT;
	if (region->flags&MMAP_REGION_FLAG_VMM_READWRITE){
		out|=VMM_PAGE_FLAG_READWRITE;
	}
	if (region->flags&MMAP_REGION_FLAG_VMM_USER){
		out|=VMM_PAGE_FLAG_USER;
	}
	if (region->flags&MMAP_REGION_FLAG_VMM_NOEXECUTE){
		out|=VMM_PAGE_FLAG_NOEXECUTE;
	}
	return out;
}



u64 syscall_memory_map(u64 size,u64 flags,handle_id_t fd){
	u64 mmap_flags=MMAP_REGION_FLAG_VMM_USER;
	vfs_node_t* file=NULL;
	if (flags&USER_MEMORY_FLAG_WRITE){
		mmap_flags|=MMAP_REGION_FLAG_VMM_READWRITE;
	}
	if (!(flags&USER_MEMORY_FLAG_EXEC)){
		mmap_flags|=MMAP_REGION_FLAG_VMM_NOEXECUTE;
	}
	if (flags&USER_MEMORY_FLAG_FILE){
		file=fd_get_node(fd);
		if (!file){
			return 0;
		}
	}
	if (flags&USER_MEMORY_FLAG_NOWRITEBACK){
		mmap_flags|=MMAP_REGION_FLAG_NO_FILE_WRITEBACK;
	}
	mmap_region_t* out=mmap_alloc(&(THREAD_DATA->process->mmap),0,pmm_align_up_address(size),_mmap_user_data_pmm_counter,mmap_flags,file);
	return (out?out->rb_node.key:0);
}



u64 syscall_memory_change_flags(u64 address,u64 size,u64 flags){
	u64 mmap_flags=0;
	if (flags&USER_MEMORY_FLAG_WRITE){
		mmap_flags|=VMM_PAGE_FLAG_READWRITE;
	}
	if (!(flags&USER_MEMORY_FLAG_EXEC)){
		mmap_flags|=VMM_PAGE_FLAG_NOEXECUTE;
	}
	return mmap_change_flags(&(THREAD_DATA->process->mmap),pmm_align_down_address(address),pmm_align_up_address(size+(address&(PAGE_SIZE-1))),mmap_flags,VMM_PAGE_FLAG_NOEXECUTE|VMM_PAGE_FLAG_READWRITE);
}



u64 syscall_memory_unmap(u64 address,u64 size){
	return mmap_dealloc(&(THREAD_DATA->process->mmap),pmm_align_down_address(address),pmm_align_up_address(size+(address&(PAGE_SIZE-1))));
}

